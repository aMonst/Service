#include "TestService.h"
#include <stdio.h>
#include "ServiceCtrl.h"

IMPLAMENT_SERVICE_MAIN(GetSystemInfoService, CTestService)

BEGIN_SERVICE_MAP()
	ON_SERVICE_MAP(GetSystemInfoService, CTestService)
END_SERIVCE_MAP()

int _tmain(int argc, TCHAR* argv[])
{
	if (argc != 2)
	{
		CFSZService::RegisterService();
		return 0;
	}

	if (_tcscmp(_T("install"), argv[1]) == 0)
	{
		//安装服务
		TCHAR szAppPath[MAX_PATH] = _T("");
		GetModuleFileName(NULL, szAppPath, MAX_PATH);
		BOOL bRet = CServiceCtrl::InstallService(_T("GetSystemInfoService"), _T("GetSystemInfoService"), SERVICE_DEMAND_START,szAppPath);
		if (!bRet)
		{
			printf("安装服务失败\n");
			return -1;
		}

	}else if(_tcscmp(_T("start"), argv[1]) == 0)
	{
		BOOL bRet = CServiceCtrl::ControlService(_T("GetSystemInfoService"), SERVICE_START);
		if (!bRet)
		{
			printf("启动服务失败\n");
			return 0;
		}
	}else if(_tcscmp(_T("enum"), argv[1]) == 0)
	{
		vector<SERVICE_INFO> SrvList;
		CServiceCtrl::EnumServices(SrvList);
		printf("service name:\n");
		for (vector<SERVICE_INFO>::iterator it = SrvList.begin(); it != SrvList.end(); it++)
		{
			printf("%ws\n", it->m_csSrvName);
		}
	}else if(_tcscmp(_T("stop"), argv[1]) == 0)
	{

	}else if(_tcscmp(_T("delete"), argv[1]) == 0)
	{
		CServiceCtrl::DeleteService(_T("GetSystemInfoService"));
	}

	return 0;
}