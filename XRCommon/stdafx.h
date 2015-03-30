// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <vld.h>
#include <Windows.h>
#include <assert.h>
#include <tchar.h>
#define ASSERT(x) assert(x)
#include <string>
#include <algorithm>
#include <functional>
#include <d3d10.h>
//#include <dxgi.h>

#pragma comment(lib, "tinyxml2.lib")
#pragma comment(lib, "libiconv.lib")
#pragma comment(lib, "d3d10.lib")

// TODO: reference additional headers your program requires here
