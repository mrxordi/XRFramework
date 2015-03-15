// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define LOGQUIETMODE
#define TARGET_WINDOWS

#pragma comment(lib, "Ws2_32.lib")

#ifdef _DEBUG
	#pragma comment(lib, "libcurld.lib")
#else // _DEBUG
	#pragma comment(lib, "libcurl.lib")
#endif


#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <algorithm>
#include <assert.h>
#include <vector>
#include <d3d10_1.h>
#include <d3d10.h>
#include <dxgi.h>
#define BOOST_LIB_TOOLSET "vc120"
#define ASSERT(x) assert(x)



// TODO: reference additional headers your program requires here
