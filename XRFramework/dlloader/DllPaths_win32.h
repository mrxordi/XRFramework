#pragma once


#ifdef _DEBUG
#define DLL_PATH_LIBCURL "special://app/system/libcurld.dll"
#else // DEBUG
#define DLL_PATH_LIBCURL "special://app/system/libcurl.dll"
#endif
