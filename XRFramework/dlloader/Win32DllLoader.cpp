#include "stdafxf.h"
#include "Win32DllLoader.h"
#include "LoaderContainer.h"
#include "utils/StringUtils.h"
#include "log/Log.h"
#include "filesystem/SpecialProtocol.h"


Win32DllLoader::Win32DllLoader(const char *dll) : iLibraryLoader(dll)
{
	m_dllHandle = NULL;
	bIsSystemDll = false;
	LoaderContainer::RegisterDll(this);
}


Win32DllLoader::~Win32DllLoader(void)
{
	if (m_dllHandle)
		Unload();
	LoaderContainer::UnRegisterDll(this);
}

bool Win32DllLoader::Load()
{
	if (m_dllHandle != NULL)
		return true;

	std::string strFileName(GetFileName());

	std::string strDll;
	strDll = CSpecialProtocol::TranslatePath(strFileName);

	m_dllHandle = LoadLibraryEx(strDll.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

	if (!m_dllHandle)
	{
		DWORD dw = GetLastError();
		char* lpMsgBuf = NULL;
		DWORD strLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPSTR)&lpMsgBuf, 0, NULL);
		if (strLen == 0)
			strLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPSTR)&lpMsgBuf, 0, NULL);

		if (strLen != 0)
		{
			LOGERR("Failed to load \"%s\" with error %lu: \"%s\"", strFileName.c_str(), dw, lpMsgBuf);
		}
		else
			LOGERR("Failed to load \"%s\" with error %lu", strFileName.c_str(), dw);

		LocalFree(lpMsgBuf);
		return false;
	}

	return true;
}

void Win32DllLoader::Unload()
{

	if (m_dllHandle)
	{
		if (!FreeLibrary(m_dllHandle))
			LOGERR("Unable to unload %s", GetName());
	}

	m_dllHandle = NULL;
}

int Win32DllLoader::ResolveExport(const char* symbol, void** f, bool logging /*= true*/)
{
	if (!m_dllHandle && !Load())
	{
		if (logging)
			LOGWARN("Unable to resolve: %s %s, reason: DLL not loaded", GetName(), symbol);
		return 0;
	}

	void *s = GetProcAddress(m_dllHandle, symbol);

	if (!s)
	{
		if (logging)
			LOGWARN("Unable to resolve: %s %s", GetName(), symbol);
		return 0;
	}

	*f = s;
	return 1;
}

bool Win32DllLoader::IsSystemDll()
{
	return false;
}

HMODULE Win32DllLoader::GetHModule()
{
	return m_dllHandle;
}

bool Win32DllLoader::HasSymbols()
{
	return false;
}
