#pragma once
#include <windows.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <iostream>
#include <assert.h>
#include <Dbt.h>
#include <strsafe.h>
#define FACILITY_SYSTEM                  0x0
#define FACILITY_STUBS                   0x3
#define FACILITY_RUNTIME                 0x2
#define FACILITY_IO_ERROR_CODE           0x4
#define SVC_ERROR                        ((DWORD)0xC0020001L)

using namespace std;

class CFSZService
{
public:
	typedef CAtlMap<CString, CFSZService *> CFSZServiceMap;  //服务名称和对应的服务对象

	CFSZService(const CString& csSrvName);
	~CFSZService(void);
	virtual DWORD Run(DWORD dwArgc, LPTSTR* lpszArgv);
	virtual BOOL OnInitService(DWORD dwArgc, LPTSTR* lpszArgv); //初始化服务
	virtual DWORD RunService(); //运行服务 
	void SetServiceStatusHandle(SERVICE_STATUS_HANDLE);

	static DWORD WINAPI HandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
	static BOOL RegisterService();
	//服务命令处理函数
protected:
	virtual DWORD OnStop();
	virtual DWORD OnUserControl(DWORD dwControl);
	virtual DWORD OnStart();
	virtual DWORD OnContinue();
	virtual DWORD OnPause();
	virtual DWORD OnShutdown();
	virtual DWORD OnInterrogate();
	virtual DWORD OnShutDown();

protected://设备变更事件通知处理 SERVICE_CONTROL_DEVICEEVENT
	virtual DWORD OnDeviceArrival(PDEV_BROADCAST_HDR pDbh){return 0;}
	virtual DWORD OnDeviceRemoveComplete(PDEV_BROADCAST_HDR pDbh){return 0;}
	virtual DWORD OnDeviceQueryRemove(PDEV_BROADCAST_HDR pDbh){return 0;}
	virtual DWORD OnDeviceQueryRemoveFailed(PDEV_BROADCAST_HDR pDbh){return 0;}
	virtual DWORD OnDeviceRemovePending(PDEV_BROADCAST_HDR pDbh){return 0;}
	virtual DWORD OnCustomEvent(PDEV_BROADCAST_HDR pDbh){return 0;}
protected://硬件配置文件发生变动 SERVICE_CONTROL_HARDWAREPROFILECHANGE
	virtual DWORD OnConfigChanged(){return 0;}
	virtual DWORD OnQueryChangeConfig(){return 0;}
	virtual DWORD OnConfigChangeCanceled(){return 0;}
protected://设备电源事件 SERVICE_CONTROL_POWEREVENT
	virtual DWORD OnPowerSettingChange(PPOWERBROADCAST_SETTING pPs){return 0;}
protected://session 发生变化 SERVICE_CONTROL_SESSIONCHANGE
	virtual DWORD OnWTSConsoleConnect(PWTSSESSION_NOTIFICATION pWn){return 0;}
	virtual DWORD OnWTSConsoleDisconnect(PWTSSESSION_NOTIFICATION pWns){return 0;}
	virtual DWORD OnWTSRemoteConnect(PWTSSESSION_NOTIFICATION pWns){return 0;}
	virtual DWORD OnWTSRemoteDisconnect(PWTSSESSION_NOTIFICATION pWns){return 0;}
	virtual DWORD OnWTSSessionLogon(PWTSSESSION_NOTIFICATION pWns){return 0;}
	virtual DWORD OnWTSSessionLogoff(PWTSSESSION_NOTIFICATION pWns){return 0;}
	virtual DWORD OnWTSSessionLock(PWTSSESSION_NOTIFICATION pWns){return 0;}
	virtual DWORD OnWTSSessionUnLock(PWTSSESSION_NOTIFICATION pWns){return 0;}
	virtual DWORD OnWTSSessionRemoteControl(PWTSSESSION_NOTIFICATION pWns){return 0;}
protected:
	//内部的工具方法，设置服务为一个指定的状态
	BOOL SetStatus(DWORD dwStatus,DWORD dwCheckPoint = 0,DWORD dwWaitHint = 0 ,DWORD dwExitCode = 0,DWORD dwAcceptStatus = SERVICE_CONTROL_INTERROGATE);
	BOOL SetStartPending(DWORD dwCheckPoint = 0,DWORD dwWaitHint = 0); //设为正在启动状态
	BOOL SetContinuePending(DWORD dwCheckPoint = 0,DWORD dwWaitHint = 0); //设为正在继续运行状态
	BOOL SetPausePending(DWORD dwCheckPoint = 0,DWORD dwWaitHint = 0); //设为正在暂停状态
	BOOL SetPause(); //设为暂停状态
	BOOL SetRunning(); //设为以启动状态
	BOOL SetStopPending(DWORD dwCheckPoint = 0,DWORD dwWaitHint = 0); //设为正在停止状态
	BOOL SetStop(DWORD dwExitCode = 0); //设为以停止状态
	BOOL ReportStatus(DWORD, DWORD, DWORD);//向服务管理器报告当前服务状态
protected:
	CString m_csSrvName; //服务名称
	DWORD m_dwCurrentStatus; //当前状态
	SERVICE_STATUS_HANDLE m_hCtrl; //控制句柄
public:
	static CFSZServiceMap ms_SrvMap;
};

#define DECLARE_SERVICE_TABLE_ENTRY extern SERVICE_TABLE_ENTRY g_SrvTableEntry[];

#define BEGIN_SERVICE_MAP() \
	SERVICE_TABLE_ENTRY g_SrvTableEntry[] = {

#define ON_SERVICE_MAP(srvName, className)\
{_T(#srvName), _ServiceMain_##className},

#define END_SERIVCE_MAP()\
{NULL, NULL}};

#define DECLARE_SERVICE_MAIN(className) VOID WINAPI _ServiceMain_##className(DWORD dwArgc, LPTSTR* lpszArgv);
#define IMPLAMENT_SERVICE_MAIN(srvName, className)\
	VOID WINAPI _ServiceMain_##className(DWORD dwArgc, LPTSTR* lpszArgv)\
	{\
		CFSZService *pThis = NULL;\
		if(!CFSZService::ms_SrvMap.Lookup(_T(#srvName), pThis))\
		{\
			pThis = dynamic_cast<CFSZService*>( new className(_T(#srvName)) );\
		}\
		else\
		{\
			return;\
		}\
		assert(NULL != pThis);\
		SERVICE_STATUS_HANDLE hss = RegisterServiceCtrlHandlerEx(_T(#srvName), CFSZService::HandlerEx, reinterpret_cast<LPVOID>(pThis));\
		assert(NULL != hss);\
		pThis->SetServiceStatusHandle(hss);\
		pThis->Run(dwArgc, lpszArgv);\
		delete dynamic_cast<##className*>(pThis);\
	}