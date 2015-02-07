#include "stdafxf.h"
#include <delayimp.h>
#include "../XRCommon/utils/SpecialProtocol.h"
#include "../XRCommon/utils/StringUtils.h"

FARPROC WINAPI delayHookNotifyFunc(unsigned dliNotify, PDelayLoadInfo pdli)
{
	switch (dliNotify)
	{
	case dliNotePreLoadLibrary:
		if (_stricmp(pdli->szDll, "avcodec-56.dll") == 0)
		{
			std::string strDll = CSpecialProtocol::TranslatePath("special://app/system/avcodec-56.dll");
			HMODULE hMod = LoadLibraryExA(strDll.c_str(), 0, LOAD_WITH_ALTERED_SEARCH_PATH);
			return (FARPROC)hMod;
		}
		if (_stricmp(pdli->szDll, "avfilter-5.dll") == 0)
		{
			std::string strDll = CSpecialProtocol::TranslatePath("special://app/system/avfilter-5.dll");
			HMODULE hMod = LoadLibraryExA(strDll.c_str(), 0, LOAD_WITH_ALTERED_SEARCH_PATH);
			return (FARPROC)hMod;
		}
		if (_stricmp(pdli->szDll, "avformat-56.dll") == 0)
		{
			std::string strDll = CSpecialProtocol::TranslatePath("special://app/system/avformat-56.dll");
			HMODULE hMod = LoadLibraryExA(strDll.c_str(), 0, LOAD_WITH_ALTERED_SEARCH_PATH);
			return (FARPROC)hMod;
		}
		if (_stricmp(pdli->szDll, "avutil-54.dll") == 0)
		{
			std::string strDll = CSpecialProtocol::TranslatePath("special://app/system/avutil-54.dll");
			HMODULE hMod = LoadLibraryExA(strDll.c_str(), 0, LOAD_WITH_ALTERED_SEARCH_PATH);
			return (FARPROC)hMod;
		}
		if (_stricmp(pdli->szDll, "postproc-53.dll") == 0)
		{
			std::string strDll = CSpecialProtocol::TranslatePath("special://app/system/postproc-53.dll");
			HMODULE hMod = LoadLibraryExA(strDll.c_str(), 0, LOAD_WITH_ALTERED_SEARCH_PATH);
			return (FARPROC)hMod;
		}
		if (_stricmp(pdli->szDll, "swresample-1.dll") == 0)
		{
			std::string strDll = CSpecialProtocol::TranslatePath("special://app/system/swresample-1.dll");
			HMODULE hMod = LoadLibraryExA(strDll.c_str(), 0, LOAD_WITH_ALTERED_SEARCH_PATH);
			return (FARPROC)hMod;
		}
		if (_stricmp(pdli->szDll, "swscale-3.dll") == 0)
		{
			std::string strDll = CSpecialProtocol::TranslatePath("special://app/system/swscale-3.dll");
			HMODULE hMod = LoadLibraryExA(strDll.c_str(), 0, LOAD_WITH_ALTERED_SEARCH_PATH);
			return (FARPROC)hMod;
		}

	default:
		break;
	}
	return NULL;
}


FARPROC WINAPI delayHookFailureFunc(unsigned dliNotify, PDelayLoadInfo pdli)
{
	switch (dliNotify)
	{
	case dliFailLoadLib:
		std::string strError;
			strError = StringUtils::Format("Fatal Error: Uh oh, can't load %s, exiting.", pdli->szDll);
		MessageBoxA(nullptr, strError.c_str(), MB_OK, 0);
		exit(1);
		break;
	}
	return NULL;
}

// assign hook functions
PfnDliHook __pfnDliNotifyHook2 = delayHookNotifyFunc;
PfnDliHook __pfnDliFailureHook2 = delayHookFailureFunc;