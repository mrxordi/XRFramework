#pragma once

class SysInfo
{
public:
	SysInfo(void);
	virtual ~SysInfo(void);
	static std::string GetUAWindowsVersion();
	static std::string GetUserAgent();
	static BOOL IsCurrentUserLocalAdministrator();
};

