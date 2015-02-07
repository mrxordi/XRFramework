#include "stdafx.h"
#include "Util.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/SpecialProtocol.h"
#include <shlobj.h>

std::string CUtil::ValidatePath(const std::string &path, bool bFixDoubleSlashes /*= false*/)
{
	std::string result = path;

	// Don't do any stuff on URLs containing %-characters or protocols that embed
	// filenames. NOTE: Don't use IsInZip or IsInRar here since it will infinitely
	// recurse and crash XBMC
	if (URIUtils::IsURL(path) &&
		(path.find('%') != std::string::npos ||
		StringUtils::StartsWithNoCase(path, "apk:") ||
		StringUtils::StartsWithNoCase(path, "zip:") ||
		StringUtils::StartsWithNoCase(path, "rar:") ||
		StringUtils::StartsWithNoCase(path, "stack:") ||
		StringUtils::StartsWithNoCase(path, "bluray:") ||
		StringUtils::StartsWithNoCase(path, "multipath:")))
		return result;

	// check the path for incorrect slashes
	if (URIUtils::IsDOSPath(path))
	{
		StringUtils::Replace(result, '/', '\\');
		/* The double slash correction should only be used when *absolutely*
		   necessary! This applies to certain DLLs or use from Python DLLs/scripts
		   that incorrectly generate double (back) slashes.
		   */
		if (bFixDoubleSlashes && !result.empty())
		{
			// Fixup for double back slashes (but ignore the \\ of unc-paths)
			for (size_t x = 1; x < result.size() - 1; x++)
			{
				if (result[x] == '\\' && result[x + 1] == '\\')
					result.erase(x);
			}
		}
	}
	else if (path.find("://") != std::string::npos || path.find(":\\\\") != std::string::npos)
	{
		StringUtils::Replace(result, '\\', '/');
		/* The double slash correction should only be used when *absolutely*
		   necessary! This applies to certain DLLs or use from Python DLLs/scripts
		   that incorrectly generate double (back) slashes.
		   */
		if (bFixDoubleSlashes && !result.empty())
		{
			// Fixup for double forward slashes(/) but don't touch the :// of URLs
			for (size_t x = 2; x < result.size() - 1; x++)
			{
				if (result[x] == '/' && result[x + 1] == '/' && !(result[x - 1] == ':' || (result[x - 1] == '/' && result[x - 2] == ':')))
					result.erase(x);
			}
		}
	}
	return result;
}

std::string CUtil::ResolveExecutablePath()
{
	std::string strExecutablePath;

	static const size_t bufSize = MAX_PATH * 2;
	char* buf = new char[bufSize];
	buf[0] = 0;
	::GetModuleFileNameA(0, buf, bufSize);
	buf[bufSize - 1] = 0;
	strExecutablePath += buf;
	delete[] buf;

	std::string strPath;

	size_t last_sep = strExecutablePath.find_last_of(PATH_SEPARATOR_CHAR);
	if (last_sep != std::string::npos)
		strPath = strExecutablePath.substr(0, last_sep);
	else
		strPath = strExecutablePath;

	return strPath;
}

std::string CUtil::ResolveSystemTempPath()
{
	std::string strTempPath;

	static const size_t bufSize = MAX_PATH * 2;
	char* buf = new char[bufSize];
	buf[0] = 0;
	::GetTempPathA(bufSize, buf);
	buf[bufSize - 1] = 0;
	strTempPath += buf;
	delete[] buf;

	return strTempPath;
}

std::string CUtil::ResolveUserPath()
{
	std::string strUserPath;

	static const size_t bufSize = MAX_PATH * 2;
	char* buf = new char[bufSize];
	buf[0] = 0;
	::SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, buf);
	buf[bufSize - 1] = 0;
	strUserPath += buf;
	delete[] buf;

	return strUserPath;
}

std::string CUtil::ResolveDocPath()
{
	std::string strDocPath;

	static const size_t bufSize = MAX_PATH * 2;
	char* buf = new char[bufSize];
	buf[0] = 0;
	::SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, buf);
	buf[bufSize - 1] = 0;
	strDocPath += buf;
	delete[] buf;

	return strDocPath;
}

bool CUtil::IsPicture(const std::string& strFile)
{
	return URIUtils::HasExtension(strFile,
		std::string(URIUtils::pictureExtensions));
}

// Retrieve the filename of the process that currently has the focus.
// Typically this will be some process using the system tray grabbing
// the focus and causing XBMC to minimize. Logging the offending
// process name can help the user fix the problem.
bool CUtil::GetFocussedProcess(std::wstring &strProcessFile)
{
	strProcessFile = L"";

	// Get the window that has the focus
	HWND hfocus = GetForegroundWindow();
	if (!hfocus)
		return false;

	// Get the process ID from the window handle
	DWORD pid = 0;
	GetWindowThreadProcessId(hfocus, &pid);

	// Use OpenProcess to get the process handle from the process ID
	HANDLE hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, pid);
	if (!hproc)
		return false;

	// Load QueryFullProcessImageName dynamically because it isn't available
	// in all versions of Windows.
	wchar_t procfile[MAX_PATH + 1];
	DWORD procfilelen = MAX_PATH;

	HINSTANCE hkernel32 = LoadLibrary(TEXT("kernel32.dll"));
	if (hkernel32)
	{
		DWORD(WINAPI *pQueryFullProcessImageNameA)(HANDLE, DWORD, LPTSTR, PDWORD);
		pQueryFullProcessImageNameA = (DWORD(WINAPI *)(HANDLE, DWORD, LPTSTR, PDWORD)) GetProcAddress(hkernel32, "QueryFullProcessImageNameA");
		if (pQueryFullProcessImageNameA)
			if (pQueryFullProcessImageNameA(hproc, 0, procfile, &procfilelen))
				strProcessFile = procfile;
		FreeLibrary(hkernel32);
	}

	// If QueryFullProcessImageName failed fall back to GetModuleFileNameEx.
	// Note this does not work across x86-x64 boundaries.
	if (strProcessFile == L"")
	{
		HINSTANCE hpsapi = LoadLibrary(_T("psapi.dll"));
		if (hpsapi)
		{
			DWORD(WINAPI *pGetModuleFileNameExA)(HANDLE, HMODULE, LPTSTR, DWORD);
			pGetModuleFileNameExA = (DWORD(WINAPI*)(HANDLE, HMODULE, LPTSTR, DWORD)) GetProcAddress(hpsapi, "GetModuleFileNameExA");
			if (pGetModuleFileNameExA)
				if (pGetModuleFileNameExA(hproc, NULL, procfile, MAX_PATH))
					strProcessFile = procfile;
			FreeLibrary(hpsapi);
		}
	}

	CloseHandle(hproc);

	return true;
}


//
// Usage: SetThreadName (-1, "MainThread");
// Code from http://msdn.microsoft.com/en-us/library/xcb2z8hs%28v=vs.110%29.aspx
//

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO {
	DWORD  dwType;      // must be 0x1000
	LPCSTR szName;      // pointer to name (in user addr space)
	DWORD  dwThreadID;  // thread ID (-1 caller thread)
	DWORD  dwFlags;     // reserved for future use, must be zero
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try {
		RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
}