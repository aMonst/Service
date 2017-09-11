#include "TestService.h"

CTestService::CTestService(const CString& csSrvName):
	CFSZService(csSrvName),
	m_bStop(FALSE)
{
	m_hMemInfoFile = CreateFile(_T("C:\\meminfo.log"), GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

CTestService::~CTestService(void)
{
	if (INVALID_HANDLE_VALUE != m_hMemInfoFile)
	{
		CloseHandle(m_hMemInfoFile);
	}
}

DWORD CTestService::RunService()
{
	while (!m_bStop)
	{
		SYSTEM_INFO si = {0};
		GetSystemInfo(&si);
		CString csInfo;
		csInfo.Format(_T("get system information: Pagesize %d, MaximumApplicationAddress:%08x, MaximumApplicationAddress:%08x, NumberOfProcessors:%d\n"), si.dwPageSize, (DWORD)si.lpMaximumApplicationAddress, (DWORD)si.lpMaximumApplicationAddress, si.dwNumberOfProcessors);
		DWORD dwWritten = 0;
		WriteFile(m_hMemInfoFile, csInfo.GetBuffer(), csInfo.GetLength() * sizeof(TCHAR), &dwWritten, NULL);
	}

	return CFSZService::RunService();
}

DWORD CTestService::OnStop()
{
	m_bStop = TRUE;
	return CFSZService::OnStop();
}