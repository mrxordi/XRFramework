#pragma once
#include "utils/StdString.h"
class SysInfo
{
public:
	SysInfo(void);
	virtual ~SysInfo(void);
	static CStdString GetUAWindowsVersion();
	static CStdString GetUserAgent();
	static BOOL IsCurrentUserLocalAdministrator();
};

