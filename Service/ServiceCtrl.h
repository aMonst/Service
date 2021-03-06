#pragma once
#include <Windows.h>
#include <atlstr.h>
#include <vector>
#include <strsafe.h>
#include <atlcoll.h>
using namespace std;

#define MAX_DESCRIB_LENGTH 4096

typedef struct _tag_SERVICE_INFO 
{
	CString m_csSrvName;//服务名称
	CString m_csSrvDispName;//显示名称
	DWORD m_dwSrvStatus;//服务状态
	DWORD m_dwSrvStartType;//启动类型
	CString m_csBinPath;//所在路径
	CString m_csDescribInfo;//描述信息
	CString m_csaDependSev;//依赖关系
}SERVICE_INFO, *LPSERVICE_INFO;

class CServiceCtrl
{
public:
	CServiceCtrl(void);
	~CServiceCtrl(void);

	static BOOL InstallService(const CString &strSrvName, const CString &strDisName, DWORD dwStartType, LPCTSTR lpszBinaryPathName, LPCTSTR lpszSrvDesc = NULL); //安装服务到注册表中
	static BOOL EnumServices(vector<SERVICE_INFO> &SrvList);//枚举系统存在的服务
	static BOOL GetServiceCurrentStatus(const CString &csSrvName, DWORD &dwCurrentStatus);//将服务的当前状态转化为字符串
	static BOOL GetSrvBinPath(const CString &csSrvName, CString &csSrvPath);//获取服务的路径
	static BOOL GetServiceStartType(const CString &csSrvName, DWORD &dwStartType); //获取服务的启动类型
	static BOOL GetServiceDescribtion(const CString& strSrvName, CString &csDisc);//获取服务的描述信息
	static BOOL GetServiceDependence(const CString &csSrvName, CString &csaDependence); //获取它所依赖的服务
	static BOOL GetSrvCtrlAccept(const CString& strSrvName, DWORD &dwAcceptCtrl);//获取服务的可控制状态
	static BOOL ControlService(const CString& strSrvName, DWORD dwNewStatus);//更改服务状态
	static BOOL ChangeServiceStartType(const CString& strSrvName, DWORD dwNewStartType);//更改服务的启动类型
	static BOOL DeleteService(const CString &csSrvName); //删除服务
};
