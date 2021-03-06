#pragma once
#include "render\dxerr.h"
#include "log\Log.h"

#define SAFE_DELETE(p)       do { delete (p);     (p)=NULL; } while (0)
#define SAFE_DELETE_ARRAY(p) do { delete[] (p);   (p)=NULL; } while (0)
#define SAFE_RELEASE(p)      do { if(p) { (p)->Release(); (p)=NULL; } } while (0)

// Useful pixel colour manipulation macros
#define GET_A(color)            ((color >> 24) & 0xFF)
#define GET_R(color)            ((color >> 16) & 0xFF)
#define GET_G(color)            ((color >>  8) & 0xFF)
#define GET_B(color)            ((color >>  0) & 0xFF)

#ifndef HR
#define HR(x) {                                             \
	HRESULT hr = (x);                                      \
	if(FAILED(hr))                                         \
				{											\
		LOGERR("%X - %s", hr, DXGetErrorString(hr));		\
		DXTraceW(__FILEW__, (DWORD)__LINE__, hr, L#x, true); \
				}									\
		}
#endif

#if !defined(_SSIZE_T_DEFINED) && !defined(HAVE_SSIZE_T)
typedef intptr_t      ssize_t;
#define _SSIZE_T_DEFINED
#endif // !_SSIZE_T_DEFINED
#ifndef SSIZE_MAX
#define SSIZE_MAX INTPTR_MAX
#endif // !SSIZE_MAX
#define snprintf _snprintf