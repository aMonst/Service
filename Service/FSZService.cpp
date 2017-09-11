#include "FSZService.h"

CFSZService::CFSZServiceMap CFSZService::ms_SrvMap;
DECLARE_SERVICE_TABLE_ENTRY

CFSZService::CFSZService(const CString& csSrvName)
{
	m_csSrvName = csSrvName;
	ms_SrvMap.SetAt(m_csSrvName, this);
}

CFSZService::~CFSZService(void)
{
}

void CFSZService::SetServiceStatusHandle(SERVICE_STATUS_HANDLE hCtrl)
{
	m_hCtrl = hCtrl;
}

DWORD WINAPI CFSZService::HandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
	DWORD dwRet = ERROR_SUCCESS;

	if( NULL == lpContext )
	{
		return ERROR_INVALID_PARAMETER;
	}

	CFSZService*pService =  reinterpret_cast<CFSZService*>(lpContext);
	if( NULL == pService )
	{
		return ERROR_INVALID_PARAMETER;
	}

	switch(dwControl)
	{
	case SERVICE_CONTROL_STOP:	//0x00000001 停止服务器
		{
			dwRet = pService -> OnStop();
		}
		break;
	case SERVICE_CONTROL_PAUSE:	//0x00000002 暂停
		{
			dwRet = pService -> OnPause();
		}
		break;
	case SERVICE_CONTROL_CONTINUE: //0x00000003 继续
		{
			dwRet = pService -> OnContinue();
		}
		break;
	case SERVICE_CONTROL_INTERROGATE: //0x00000004 查询
		{
			dwRet = pService -> OnInterrogate();
		}
		break;
	case SERVICE_CONTROL_SHUTDOWN: //0x00000005 系统关机,做清理操作
		{
			dwRet = pService -> OnShutDown();
		}
		break;
	case SERVICE_CONTROL_DEVICEEVENT://0x0000000B
		{
			PDEV_BROADCAST_HDR pDbh = (PDEV_BROADCAST_HDR)lpEventData;
			switch(dwEventType)
			{
			case DBT_DEVICEARRIVAL:
				{
					dwRet = pService->OnDeviceArrival(pDbh);
				}
				break;
			case DBT_DEVICEREMOVECOMPLETE:
				{
					dwRet = pService->OnDeviceRemoveComplete(pDbh);
				}
				break;
			case DBT_DEVICEQUERYREMOVE:
				{
					dwRet = pService->OnDeviceQueryRemove(pDbh);
				}
				break;
			case DBT_DEVICEQUERYREMOVEFAILED:
				{
					dwRet = pService->OnDeviceQueryRemoveFailed(pDbh);
				}
				break;
			case DBT_DEVICEREMOVEPENDING:
				{
					dwRet = pService->OnDeviceRemovePending(pDbh);
				}
				break;
			case DBT_CUSTOMEVENT:
				{
					dwRet = pService->OnCustomEvent(pDbh);
				}
				break;
			default:
				dwRet = ERROR_INVALID_PARAMETER;
			}

		}
		break;
	case SERVICE_CONTROL_HARDWAREPROFILECHANGE://0x0000000C
		{
			switch(dwEventType)
			{
			case DBT_CONFIGCHANGED:
				{
					dwRet = pService->OnConfigChanged();
				}
				break;
			case DBT_QUERYCHANGECONFIG:
				{
					dwRet = pService->OnQueryChangeConfig();
				}
				break;
			case DBT_CONFIGCHANGECANCELED:
				{
					dwRet = pService->OnConfigChangeCanceled();
				}
				break;
			default:
				dwRet = ERROR_INVALID_PARAMETER;
			}

		}
		break;
	case SERVICE_CONTROL_POWEREVENT://0x0000000D
		{//Notifies a service of system power events.
			switch(dwEventType)
			{
			case PBT_POWERSETTINGCHANGE:
				{
					PPOWERBROADCAST_SETTING pPs = (PPOWERBROADCAST_SETTING)lpEventData;
					dwRet = pService->OnPowerSettingChange(pPs);
				}
				break;
			default:
				dwRet = ERROR_INVALID_PARAMETER;
			}
		}
		break;
	case SERVICE_CONTROL_SESSIONCHANGE://0x0000000E
		{
			PWTSSESSION_NOTIFICATION pWn = (PWTSSESSION_NOTIFICATION)lpEventData;
			switch(dwEventType)
			{
			case WTS_CONSOLE_CONNECT://0x1
				{
					dwRet = pService->OnWTSConsoleConnect(pWn);
				}
				break;
			case WTS_CONSOLE_DISCONNECT://0x2
				{
					dwRet = pService->OnWTSConsoleDisconnect(pWn);
				}
				break;
			case WTS_REMOTE_CONNECT://0x3
				{
					dwRet = pService->OnWTSRemoteConnect(pWn);
				}
				break;
			case WTS_REMOTE_DISCONNECT://0x4
				{
					dwRet = pService->OnWTSRemoteDisconnect(pWn);
				}
				break;
			case WTS_SESSION_LOGON://0x5
				{
					dwRet = pService->OnWTSSessionLogon(pWn);
				}
				break; 
			case WTS_SESSION_LOGOFF://0x6
				{
					dwRet = pService->OnWTSSessionLogoff(pWn);
				}
				break;
			case WTS_SESSION_LOCK://0x7
				{
					dwRet = pService->OnWTSSessionLock(pWn);
				}
				break;
			case WTS_SESSION_UNLOCK://0x8
				{
					dwRet = pService->OnWTSSessionUnLock(pWn);
				}
				break;
			case WTS_SESSION_REMOTE_CONTROL://0x9
				{
					dwRet = pService->OnWTSSessionRemoteControl(pWn);
				}
				break;
			default:
				dwRet = ERROR_INVALID_PARAMETER;
			}
			break;
		}
	default:		//自定义的操作码(128 to 255)
		if(127 < dwControl && 256 > dwControl)
		{
			dwRet = pService -> OnUserControl(dwControl);
		}
		else
		{
			return ERROR_CALL_NOT_IMPLEMENTED;
		}
		break;
	}

	return dwRet;
}

BOOL CFSZService::SetStatus(DWORD dwStatus,DWORD dwCheckPoint,DWORD dwWaitHint, DWORD dwExitCode,DWORD dwAcceptStatus)
{
	SERVICE_STATUS ss = {0};
	ss.dwCheckPoint = dwCheckPoint;
	ss.dwWaitHint = dwWaitHint;
	ss.dwWin32ExitCode = dwExitCode;
	ss.dwControlsAccepted = dwAcceptStatus;
	ss.dwCurrentState = dwStatus;
	m_dwCurrentStatus = dwStatus;

	return ::SetServiceStatus(m_hCtrl, &ss);
}

BOOL CFSZService::SetStartPending(DWORD dwCheckPoint,DWORD dwWaitHint)
{
	return SetStatus(SERVICE_START_PENDING, dwCheckPoint, dwWaitHint);
}

BOOL CFSZService::SetContinuePending(DWORD dwCheckPoint,DWORD dwWaitHint)
{
	return SetStatus(SERVICE_CONTINUE_PENDING, dwCheckPoint, dwWaitHint);
}

BOOL CFSZService::SetPausePending(DWORD dwCheckPoint,DWORD dwWaitHint)
{
	return SetStatus(SERVICE_PAUSE_PENDING, dwCheckPoint, dwWaitHint);
}

BOOL CFSZService::SetPause()
{
	return SetStatus(SERVICE_PAUSED);
}

BOOL CFSZService::SetRunning()
{
	return SetStatus(SERVICE_RUNNING);
}

BOOL CFSZService::SetStopPending(DWORD dwCheckPoint,DWORD dwWaitHint)
{
	return SetStatus(SERVICE_STOP_PENDING, dwCheckPoint, dwWaitHint);
}

BOOL CFSZService::SetStop(DWORD dwExitCode)
{
	return SetStatus(SERVICE_STOP, dwExitCode);
}

DWORD CFSZService::Run(DWORD dwArgc, LPTSTR* lpszArgv)
{
	assert(NULL != this);
	if (OnInitService(dwArgc, lpszArgv))
	{
		RunService();
	}
	return 0;
}

DWORD CFSZService::OnStop()
{
	return 0;
}

DWORD CFSZService::OnUserControl(DWORD dwControl)
{
	return 0;
}

DWORD CFSZService::OnStart()
{
	return 0;
}

DWORD CFSZService::OnContinue()
{
	SetRunning();
	return 0;
}

DWORD CFSZService::OnPause()
{
	SetPause();
	return 0;
}

DWORD CFSZService::OnShutdown()
{
	return 0;
}

DWORD CFSZService::OnInterrogate()
{
	SetStatus(m_dwCurrentStatus); //将当前状态报告给服务控制管理器
	return 0;
}

DWORD CFSZService::OnShutDown()
{
	return 0;
}

BOOL CFSZService::OnInitService(DWORD dwArgc, LPTSTR* lpszArgv)
{
	return TRUE;
}

DWORD CFSZService::RunService()
{
	return 0;
}

BOOL CFSZService::RegisterService()
{
	return ::StartServiceCtrlDispatcher(g_SrvTableEntry);
}