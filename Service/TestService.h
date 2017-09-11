#pragma once
#include "FSZService.h"

class CTestService : public CFSZService 
{
public:
	CTestService(const CString& csSrvName);
	~CTestService(void);

	virtual DWORD RunService();
	virtual DWORD OnStop();
protected:
	HANDLE m_hMemInfoFile;
	BOOL m_bStop;
};

DECLARE_SERVICE_TABLE_ENTRY
