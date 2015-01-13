#include "stdafxf.h"
#include "RenderControl.h"
#include "render/RenderSystemDX.h"
#include "Windowsx.h"


RenderControl* RenderControl::m_pThis = nullptr;
WNDPROC RenderControl::m_OldWndProc = nullptr;


RenderControl::RenderControl() : w(0), h(0)
{
}


RenderControl::~RenderControl()
{
}

HWND RenderControl::Initialize(HWND parent, HINSTANCE hInst, RECT rc) {

	WNDCLASSA wc;
	GetClassInfoA(hInst, "static", &wc);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "RenderControl";
	wc.hInstance = hInst;
	m_OldWndProc = wc.lpfnWndProc;
	wc.lpfnWndProc = RenderControl::NewWndProc;
	if (!RegisterClassA(&wc))
	{
		MessageBoxA(parent, "Nie uda³o siê zarejestrowaæ nowej klasy.", "Yh...", MB_ICONSTOP);
		DestroyWindow(parent);
	}

	m_hWnd = CreateWindowExA(0,                 //extended styles
		"RenderControl",                 //control 'class' name
		"static bla bla bla",         //control caption
		WS_CHILD | WS_VISIBLE,        //control style 
		rc.left,                      //position: left
		rc.top,                       //position: top
		rc.right,                     //width
		rc.bottom,                    //height
		parent,                     //parent window handle
		//control's ID
		NULL,
		hInst,                        //application instance
		this);                           //user defined info

	w = rc.right - rc.left;
	h = rc.bottom - rc.top;

	return m_hWnd;
}

LRESULT CALLBACK RenderControl::NewWndProc(HWND hwnd, UINT mesg, WPARAM wParam, LPARAM lParam)
{
	switch (mesg) {
	case WM_PAINT:

	case WM_CREATE:
		if (!RenderControl::m_pThis)
			RenderControl::m_pThis = (RenderControl*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		break;
	case WM_SIZE:
		// Save the new client area dimensions.
		m_pThis->w = GET_X_LPARAM(lParam);
		m_pThis->h = GET_Y_LPARAM(lParam);
		g_DXRendererPtr->OnResize();

		

	case WM_DESTROY: {// do g³ównej procedury okna
		PostQuitMessage(0);
		break;
	}
	}


	return CallWindowProc(m_OldWndProc, hwnd, mesg, wParam, lParam);
}
