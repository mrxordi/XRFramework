#pragma once
#include <windows.h>
#include "../IApplication.h"
//#include "WEvent.h"


typedef bool(*PHANDLE_EVENT_FUNC)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CWindowEvents
{
public:
	static bool MessagePump();
	static size_t GetQueueSize();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static IApplication* m_pIApplication;

};

