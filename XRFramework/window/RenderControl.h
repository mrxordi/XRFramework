#pragma once
#include <Windows.h>
#include "Windowsx.h"


class RenderControl
{
public:
	RenderControl();
	~RenderControl();

	HWND Initialize(HWND parent, HINSTANCE hInst, RECT rc);

	static LRESULT CALLBACK NewWndProc(HWND hwnd, UINT mesg, WPARAM wParam, LPARAM lParam);

	void GetWH(int& wi, int& he) {
		wi = w;
		he = h;
	}

	HWND GetHWND() { return m_hWnd; }

private:
	HWND m_hWnd;
	RECT m_rect;
	int w, h;

	static RenderControl* m_pThis;
	static WNDPROC m_OldWndProc;

};

