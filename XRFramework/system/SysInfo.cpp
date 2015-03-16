#include "stdafxf.h"
#include "SysInfo.h"
#include "utils/StringUtils.h"


SysInfo::SysInfo(void)
{
}


SysInfo::~SysInfo(void)
{
}

std::string SysInfo::GetUserAgent()
{
	std::string result;
	result = "XRLib/ (";
	result += GetUAWindowsVersion();
	result += "; http://XRFramework.com)";

	return result;
}

std::string SysInfo::GetUAWindowsVersion()
{
	OSVERSIONINFOEX osvi = {};

	osvi.dwOSVersionInfoSize = sizeof(osvi);
	std::string strVersion = "Windows NT";

	if (GetVersionEx((LPOSVERSIONINFO)&osvi))
	{
		strVersion += StringUtils::Format(" %d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion);
	}

	SYSTEM_INFO si = {};
	GetSystemInfo(&si);

	BOOL bIsWow = FALSE;
	if (IsWow64Process(GetCurrentProcess(), &bIsWow))
	{
		if (bIsWow)
		{
			strVersion.append(";WOW64");
			GetNativeSystemInfo(&si);     // different function to read the info under Wow
		}
	}

	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
		strVersion.append(";Win64;x64");
	else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
		strVersion.append(";Win64;IA64");

	return strVersion;
}

BOOL SysInfo::IsCurrentUserLocalAdministrator()
{
	BOOL b;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	b = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&AdministratorsGroup);
	if (b)
	{
		if (!CheckTokenMembership(NULL, AdministratorsGroup, &b))
		{
			b = FALSE;
		}
		FreeSid(AdministratorsGroup);
	}

	return(b);
}