#include "stdafxf.h"
#include "Util.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "filesystem/SpecialProtocol.h"
#include <shlobj.h>



CStdString CUtil::ValidatePath(const CStdString &path, bool bFixDoubleSlashes /*= false*/)
{
	  CStdString result = path;

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
      StringUtils::StartsWithNoCase(path, "multipath:") ))
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
        if (result[x] == '\\' && result[x+1] == '\\')
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
        if ( result[x] == '/' && result[x + 1] == '/' && !(result[x - 1] == ':' || (result[x - 1] == '/' && result[x - 2] == ':')) )
          result.erase(x);
      }
    }
  }
  return result;
}

CStdString CUtil::ResolveExecutablePath()
{
	CStdString strExecutablePath;

	static const size_t bufSize = MAX_PATH * 2;
	char* buf = new char[bufSize];
	buf[0] = 0;
	::GetModuleFileName(0, buf, bufSize);
	buf[bufSize-1] = 0;
	strExecutablePath += buf;
	delete[] buf;

	CStdString strPath;

	size_t last_sep = strExecutablePath.find_last_of(PATH_SEPARATOR_CHAR);
	if (last_sep != std::string::npos)
		strPath = strExecutablePath.substr(0, last_sep);
	else
		strPath = strExecutablePath;

	return strPath;
}

CStdString CUtil::ResolveSystemTempPath()
{
	CStdString strTempPath;

	static const size_t bufSize = MAX_PATH * 2;
	char* buf = new char[bufSize];
	buf[0] = 0;
	::GetTempPath(bufSize, buf);
	buf[bufSize-1] = 0;
	strTempPath += buf;
	delete[] buf;

	return strTempPath;
}

CStdString CUtil::ResolveUserPath()
{
	CStdString strUserPath;

	static const size_t bufSize = MAX_PATH * 2;
	char* buf = new char[bufSize];
	buf[0] = 0;
	::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, buf);
	buf[bufSize-1] = 0;
	strUserPath += buf;
	delete[] buf;

	return strUserPath;
}

CStdString CUtil::ResolveDocPath()
{
	CStdString strDocPath;

	static const size_t bufSize = MAX_PATH * 2;
	char* buf = new char[bufSize];
	buf[0] = 0;
	::SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, buf);
	buf[bufSize-1] = 0;
	strDocPath += buf;
	delete[] buf;

	return strDocPath;
}

// Retrieve the filename of the process that currently has the focus.
// Typically this will be some process using the system tray grabbing
// the focus and causing XBMC to minimise. Logging the offending
// process name can help the user fix the problem.
bool CUtil::GetFocussedProcess(std::string &strProcessFile)
{
	strProcessFile = "";

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
	char procfile[MAX_PATH+1];
	DWORD procfilelen = MAX_PATH;

	HINSTANCE hkernel32 = LoadLibrary("kernel32.dll");
	if (hkernel32)
	{
		DWORD (WINAPI *pQueryFullProcessImageNameA)(HANDLE,DWORD,LPTSTR,PDWORD);
		pQueryFullProcessImageNameA = (DWORD (WINAPI *)(HANDLE,DWORD,LPTSTR,PDWORD)) GetProcAddress(hkernel32, "QueryFullProcessImageNameA");
		if (pQueryFullProcessImageNameA)
			if (pQueryFullProcessImageNameA(hproc, 0, procfile, &procfilelen))
				strProcessFile = procfile;
		FreeLibrary(hkernel32);
	}

	// If QueryFullProcessImageName failed fall back to GetModuleFileNameEx.
	// Note this does not work across x86-x64 boundaries.
	if (strProcessFile == "")
	{
		HINSTANCE hpsapi = LoadLibrary("psapi.dll");
		if (hpsapi)
		{
			DWORD (WINAPI *pGetModuleFileNameExA)(HANDLE,HMODULE,LPTSTR,DWORD);
			pGetModuleFileNameExA = (DWORD (WINAPI*)(HANDLE,HMODULE,LPTSTR,DWORD)) GetProcAddress(hpsapi, "GetModuleFileNameExA");
			if (pGetModuleFileNameExA)
				if (pGetModuleFileNameExA(hproc, NULL, procfile, MAX_PATH))
					strProcessFile = procfile;
			FreeLibrary(hpsapi);
		}
	}

	CloseHandle(hproc);

	return true;
}

