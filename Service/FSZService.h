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
	typedef CAtlMap<CString, CFSZService *> CFSZServiceMap;  //�������ƺͶ�Ӧ�ķ������

	CFSZService(const CString& csSrvName);
	~CFSZService(void);
	virtual DWORD Run(DWORD dwArgc, LPTSTR* lpszArgv);
	virtual BOOL OnInitService(DWORD dwArgc, LPTSTR* lpszArgv); //��ʼ������
	virtual DWORD RunService(); //���з��� 
	void SetServiceStatusHandle(SERVICE_STATUS_HANDLE);

	static DWORD WINAPI HandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
	static BOOL RegisterService();
	//�����������
protected:
	virtual DWORD OnStop();
	virtual DWORD OnUserControl(DWORD dwControl);
	virtual DWORD OnStart();
	virtual DWORD OnContinue();
	virtual DWORD OnPause();
	virtual DWORD OnShutdown();
	virtual DWORD OnInterrogate();
	virtual DWORD OnShutDown();

protected://�豸����¼�֪ͨ���� SERVICE_CONTROL_DEVICEEVENT
	virtual DWORD OnDeviceArrival(PDEV_BROADCAST_HDR pDbh){return 0;}
	virtual DWORD OnDeviceRemoveComplete(PDEV_BROADCAST_HDR pDbh){return 0;}
	virtual DWORD OnDeviceQueryRemove(PDEV_BROADCAST_HDR pDbh){return 0;}
	virtual DWORD OnDeviceQueryRemoveFailed(PDEV_BROADCAST_HDR pDbh){return 0;}
	virtual DWORD OnDeviceRemovePending(PDEV_BROADCAST_HDR pDbh){return 0;}
	virtual DWORD OnCustomEvent(PDEV_BROADCAST_HDR pDbh){return 0;}
protected://Ӳ�������ļ������䶯 SERVICE_CONTROL_HARDWAREPROFILECHANGE
	virtual DWORD OnConfigChanged(){return 0;}
	virtual DWORD OnQueryChangeConfig(){return 0;}
	virtual DWORD OnConfigChangeCanceled(){return 0;}
protected://�豸��Դ�¼� SERVICE_CONTROL_POWEREVENT
	virtual DWORD OnPowerSettingChange(PPOWERBROADCAST_SETTING pPs){return 0;}
protected://session �����仯 SERVICE_CONTROL_SESSIONCHANGE
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
	//�ڲ��Ĺ��߷��������÷���Ϊһ��ָ����״̬
	BOOL SetStatus(DWORD dwStatus,DWORD dwCheckPoint = 0,DWORD dwWaitHint = 0 ,DWORD dwExitCode = 0,DWORD dwAcceptStatus = SERVICE_CONTROL_INTERROGATE);
	BOOL SetStartPending(DWORD dwCheckPoint = 0,DWORD dwWaitHint = 0); //��Ϊ��������״̬
	BOOL SetContinuePending(DWORD dwCheckPoint = 0,DWORD dwWaitHint = 0); //��Ϊ���ڼ�������״̬
	BOOL SetPausePending(DWORD dwCheckPoint = 0,DWORD dwWaitHint = 0); //��Ϊ������ͣ״̬
	BOOL SetPause(); //��Ϊ��ͣ״̬
	BOOL SetRunning(); //��Ϊ������״̬
	BOOL SetStopPending(DWORD dwCheckPoint = 0,DWORD dwWaitHint = 0); //��Ϊ����ֹͣ״̬
	BOOL SetStop(DWORD dwExitCode = 0); //��Ϊ��ֹͣ״̬
	BOOL ReportStatus(DWORD, DWORD, DWORD);//�������������浱ǰ����״̬
protected:
	CString m_csSrvName; //��������
	DWORD m_dwCurrentStatus; //��ǰ״̬
	SERVICE_STATUS_HANDLE m_hCtrl; //���ƾ��
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