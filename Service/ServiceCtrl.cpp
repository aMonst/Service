#include "ServiceCtrl.h"

CServiceCtrl::CServiceCtrl(void)
{
}

CServiceCtrl::~CServiceCtrl(void)
{
}

BOOL CServiceCtrl::InstallService(const CString &strSrvName, const CString &strDisName, DWORD dwStartType, LPCTSTR lpszBinaryPathName, LPCTSTR lpszSrvDesc)
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hSCManager)
	{
		return FALSE;
	}

	SC_HANDLE hSCRet  = ::CreateService(hSCManager, strSrvName, strDisName, SC_MANAGER_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, dwStartType, 0, lpszBinaryPathName, NULL, NULL, NULL, NULL, NULL);
	if (NULL == hSCRet)
	{
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	//如果描述信息不为空则设置描述信息
	BOOL bRet = FALSE;
	if (NULL != lpszSrvDesc)
	{
		SERVICE_DESCRIPTION sd = {0};
		TCHAR szBuf[MAX_DESCRIB_LENGTH] = _T("");
		sd.lpDescription = szBuf;
		StringCchCopy(sd.lpDescription, MAX_DESCRIB_LENGTH, lpszSrvDesc);
		bRet = ChangeServiceConfig2(hSCRet, SERVICE_CONFIG_DESCRIPTION, &sd);
		if (!bRet)
		{
			CloseServiceHandle(hSCRet);
			CloseServiceHandle(hSCManager);
			return FALSE;
		}
	}

	CloseServiceHandle(hSCRet);
	CloseServiceHandle(hSCManager);

	return TRUE;
}

BOOL CServiceCtrl::EnumServices(vector<SERVICE_INFO> &SrvList)
{
	if (!SrvList.empty())
	{
		SrvList.clear();
	}

	SC_HANDLE  hSChandle = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hSChandle)
	{
		return FALSE;
	}

	LPENUM_SERVICE_STATUS lpServices = NULL;
	DWORD cbBytesNeeded = 0;
	DWORD ServicesReturned = 0;
	DWORD ResumeHandle = 0;
	DWORD cbBufSize = 0;
	BOOL bRet = ::EnumServicesStatus(hSChandle, SERVICE_WIN32, SERVICE_STATE_ALL, lpServices, 0, &cbBytesNeeded, &ServicesReturned, &ResumeHandle);
	if (!bRet && ERROR_MORE_DATA == GetLastError())
	{
		cbBufSize = cbBytesNeeded;
		lpServices = (LPENUM_SERVICE_STATUS)new char[cbBufSize];
		::ZeroMemory(lpServices, cbBufSize);
		bRet = ::EnumServicesStatus(hSChandle, SERVICE_WIN32, SERVICE_STATE_ALL, lpServices, cbBufSize, &cbBytesNeeded, &ServicesReturned, &ResumeHandle);
		if (!bRet)
		{
			::CloseServiceHandle(hSChandle);
			return FALSE;
		}
	}

	for (int i = 0; i < ServicesReturned; i++)
	{
		SERVICE_INFO si;
		si.m_csSrvName = lpServices[i].lpServiceName;
		si.m_csSrvDispName = lpServices[i].lpDisplayName;
		si.m_dwSrvStatus = lpServices[i].ServiceStatus.dwCurrentState;
		

		//获取服务的其他信息
		GetServiceDescribtion(si.m_csSrvName, si.m_csDescribInfo);
		GetServiceStartType(si.m_csSrvName, si.m_dwSrvStartType);
		GetSrvBinPath(si.m_csSrvName, si.m_csBinPath);
		GetServiceDependence(si.m_csSrvName, si.m_csaDependSev);

		SrvList.push_back(si);
	}
	
	delete[] lpServices;
	return TRUE;
}

BOOL CServiceCtrl::GetServiceCurrentStatus(const CString &csSrvName, DWORD &dwCurrentStatus)
{
	BOOL bRet = FALSE;
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hSCManager)
	{
		return FALSE;
	}

	SC_HANDLE hSCService = OpenService(hSCManager, csSrvName, SERVICE_ALL_ACCESS);
	if (NULL == hSCService)
	{
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	SERVICE_STATUS ss = {0};
	if(!QueryServiceStatus(hSCService, &ss))
	{
		CloseServiceHandle(hSCService);
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	dwCurrentStatus = ss.dwCurrentState;
	CloseServiceHandle(hSCService);
	CloseServiceHandle(hSCManager);
	return TRUE;
}

BOOL CServiceCtrl::GetSrvBinPath(const CString &csSrvName, CString &csSrvPath)
{
	BOOL bRet = FALSE;
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hSCManager)
	{
		return FALSE;
	}

	SC_HANDLE hSCService = OpenService(hSCManager, csSrvName, SERVICE_ALL_ACCESS);
	if (NULL == hSCService)
	{
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	LPQUERY_SERVICE_CONFIG lpServiceInfo = NULL;
	DWORD dwBufferSize = 0;
	if(!QueryServiceConfig(hSCService, lpServiceInfo, dwBufferSize, &dwBufferSize))
	{
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
		{
			lpServiceInfo = (LPQUERY_SERVICE_CONFIG)new char[dwBufferSize];
			bRet = QueryServiceConfig(hSCService, lpServiceInfo, dwBufferSize, &dwBufferSize);
		}
	}

	if (bRet)
	{
		CloseServiceHandle(hSCManager);
		CloseServiceHandle(hSCService);
		return FALSE;
	}

	csSrvPath = lpServiceInfo->lpBinaryPathName;
	delete[] lpServiceInfo;
	lpServiceInfo = NULL;
	return TRUE;
}

BOOL CServiceCtrl::GetServiceStartType(const CString &csSrvName, DWORD &dwStartType)
{
	BOOL bRet = FALSE;
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hSCManager)
	{
		return FALSE;
	}

	SC_HANDLE hSCService = OpenService(hSCManager, csSrvName, SERVICE_ALL_ACCESS);
	if (NULL == hSCService)
	{
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	LPQUERY_SERVICE_CONFIG lpServiceInfo = NULL;
	DWORD dwBufferSize = 0;
	if(!QueryServiceConfig(hSCService, lpServiceInfo, dwBufferSize, &dwBufferSize))
	{
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
		{
			lpServiceInfo = (LPQUERY_SERVICE_CONFIG)new char[dwBufferSize];
			bRet = QueryServiceConfig(hSCService, lpServiceInfo, dwBufferSize, &dwBufferSize);
		}
	}

	if (bRet)
	{
		CloseServiceHandle(hSCManager);
		CloseServiceHandle(hSCService);
		return FALSE;
	}

	dwStartType = lpServiceInfo->dwStartType;
	delete[] lpServiceInfo;
	lpServiceInfo = NULL;
	return TRUE;
}
BOOL CServiceCtrl::GetServiceDescribtion(const CString& csSrvName, CString &csDisc)
{
	BOOL bRet = FALSE;
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hSCManager)
	{
		return FALSE;
	}

	SC_HANDLE hSCService = OpenService(hSCManager, csSrvName, SERVICE_ALL_ACCESS);
	if (NULL == hSCService)
	{
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	LPSERVICE_DESCRIPTION lpServiceDescription = NULL;
	DWORD dwBufferSize = 0;
	if(!QueryServiceConfig2(hSCService, SERVICE_CONFIG_DESCRIPTION, (LPBYTE)lpServiceDescription, dwBufferSize, &dwBufferSize))
	{
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
		{
			lpServiceDescription = (LPSERVICE_DESCRIPTION)new char[dwBufferSize];
			bRet = QueryServiceConfig2(hSCService, SERVICE_CONFIG_DESCRIPTION, (LPBYTE)lpServiceDescription, dwBufferSize, &dwBufferSize);
		}
	}

	if (bRet)
	{
		CloseServiceHandle(hSCManager);
		CloseServiceHandle(hSCService);
		return FALSE;
	}

	csDisc = lpServiceDescription->lpDescription;
	delete[] lpServiceDescription;
	lpServiceDescription = NULL;
	return TRUE;
	return FALSE;
}

BOOL CServiceCtrl::GetSrvCtrlAccept(const CString& csSrvName, DWORD &dwAcceptCtrl)
{
	BOOL bRet = FALSE;
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hSCManager)
	{
		return FALSE;
	}

	SC_HANDLE hSCService = OpenService(hSCManager, csSrvName, SERVICE_ALL_ACCESS);
	if (NULL == hSCService)
	{
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	SERVICE_STATUS ss = {0};
	if(!QueryServiceStatus(hSCService, &ss))
	{
		CloseServiceHandle(hSCService);
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	dwAcceptCtrl = ss.dwControlsAccepted;
	CloseServiceHandle(hSCService);
	CloseServiceHandle(hSCManager);
	return TRUE;
}

BOOL CServiceCtrl::ControlService(const CString& csSrvName, DWORD dwNewStatus)
{
	BOOL bRet = FALSE;
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hSCManager)
	{
		return FALSE;
	}

	SC_HANDLE hSCService = OpenService(hSCManager, csSrvName, SERVICE_ALL_ACCESS);
	if (NULL == hSCService)
	{
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	SERVICE_STATUS sts = {0};
	if(!QueryServiceStatus(hSCService, &sts))
	{
		CloseServiceHandle(hSCService);
		CloseServiceHandle(hSCManager);
		return bRet;
	}

	if (sts.dwCurrentState == dwNewStatus)//当前状态与修改状态一致
	{
		CloseServiceHandle(hSCService);
		CloseServiceHandle(hSCManager);
		return TRUE;
	}else if (SERVICE_START_PENDING == sts.dwCurrentState || 
		SERVICE_STOP_PENDING == sts.dwCurrentState || 
		SERVICE_CONTINUE_PENDING == sts.dwCurrentState || 
		SERVICE_PAUSE_PENDING == sts.dwCurrentState)
	{
		CloseServiceHandle(hSCService);
		CloseServiceHandle(hSCManager);
		return FALSE;
	}else if (SERVICE_STOPPED == sts.dwCurrentState && SERVICE_START == dwNewStatus)//当前状态为停止，需要启动服务
	{
		bRet = ::StartService(hSCService, NULL, NULL);
		CloseServiceHandle(hSCService);
		CloseServiceHandle(hSCManager);
		return bRet;
	}else if ((SERVICE_RUNNING == sts.dwCurrentState || SERVICE_PAUSED == sts.dwCurrentState) && SERVICE_STOP == dwNewStatus)//当前状态为已启动或者暂停，需要停止服务
	{
		bRet = ::ControlService(hSCService, SERVICE_CONTROL_STOP, &sts);
		CloseServiceHandle(hSCService);
		CloseServiceHandle(hSCManager);
		return bRet;
	}else if (SERVICE_PAUSED == sts.dwCurrentState && SERVICE_START == dwNewStatus)//当前状态为暂停，需要继续运行服务
	{
		bRet = ::ControlService(hSCService, SERVICE_CONTROL_CONTINUE, &sts);
		CloseServiceHandle(hSCService);
		CloseServiceHandle(hSCManager);
		return bRet;
	}else if (SERVICE_RUNNING == sts.dwCurrentState && SERVICE_PAUSED == dwNewStatus)//当前状态为已启动，需要修改为暂停
	{
		bRet = ::ControlService(hSCService, SERVICE_CONTROL_PAUSE, &sts);
		CloseServiceHandle(hSCService);
		CloseServiceHandle(hSCManager);
		return bRet;
	}

	return FALSE;
}

BOOL CServiceCtrl::ChangeServiceStartType(const CString& csSrvName, DWORD dwNewStartType)
{
	BOOL bRet = FALSE;
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hSCManager)
	{
		return FALSE;
	}

	SC_HANDLE hSCService = OpenService(hSCManager, csSrvName, SERVICE_ALL_ACCESS);
	if (NULL == hSCService)
	{
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	bRet = ChangeServiceConfig(hSCService, SERVICE_NO_CHANGE , dwNewStartType, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	CloseServiceHandle(hSCService);
	CloseServiceHandle(hSCManager);

	return bRet;
}

BOOL CServiceCtrl::DeleteService(const CString &csSrvName)
{
	DWORD dwCurrentStatus = 0;
	if (!GetServiceCurrentStatus(csSrvName, dwCurrentStatus))
	{
		return FALSE;
	}

	if (SERVICE_STOP != dwCurrentStatus) //如果服务不为停止状态，则停止服务
	{
		if(ControlService(csSrvName, SERVICE_STOP))
		{
			return FALSE;
		}
	}
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hSCManager)
	{
		return FALSE;
	}

	SC_HANDLE hSCService = OpenService(hSCManager, csSrvName, SERVICE_ALL_ACCESS);
	if (NULL == hSCService)
	{
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	BOOL bRet = ::DeleteService(hSCService);
	CloseServiceHandle(hSCService);
	CloseServiceHandle(hSCManager);

	return bRet;
}

BOOL CServiceCtrl::GetServiceDependence(const CString &csSrvName, CString &csaDependence)
{
	BOOL bRet = FALSE;
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hSCManager)
	{
		return FALSE;
	}

	SC_HANDLE hSCService = OpenService(hSCManager, csSrvName, SERVICE_ALL_ACCESS);
	if (NULL == hSCService)
	{
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	LPQUERY_SERVICE_CONFIG lpServiceInfo = NULL;
	DWORD dwBufferSize = 0;
	if(!QueryServiceConfig(hSCService, lpServiceInfo, dwBufferSize, &dwBufferSize))
	{
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
		{
			lpServiceInfo = (LPQUERY_SERVICE_CONFIG)new char[dwBufferSize];
			bRet = QueryServiceConfig(hSCService, lpServiceInfo, dwBufferSize, &dwBufferSize);
		}
	}

	if (bRet)
	{
		CloseServiceHandle(hSCManager);
		CloseServiceHandle(hSCService);
		return FALSE;
	}

	csaDependence = lpServiceInfo->lpDependencies;
	delete[] lpServiceInfo;
	lpServiceInfo = NULL;
	return TRUE;
}