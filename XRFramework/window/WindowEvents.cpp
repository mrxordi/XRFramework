#include "stdafxf.h"
#include "WindowEvents.h"
#include "Windowsx.h"
#include "IApplication.h"
#include "Util.h"
#include "log/Log.h"


HWND g_hWnd = NULL;

IApplication* CWindowEvents::m_pIApplication = nullptr;

bool CWindowEvents::MessagePump()
{
	MSG  msg;
	while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

size_t CWindowEvents::GetQueueSize()
{
	MSG  msg;
	return PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
}

LRESULT CALLBACK CWindowEvents::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//static bool resizing, resized, moved, minimized, maximized = false;
	//static int w, h;
	//static int rw, rh;

	if (uMsg == WM_CREATE) {
		g_hWnd = hWnd;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)(((LPCREATESTRUCT)lParam)->lpCreateParams));
		m_pIApplication = (IApplication*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		m_pIApplication->OnCreate(hWnd);
		return 0;
	}


	if (uMsg == WM_DESTROY)
		g_hWnd = NULL;

	//m_pIApplication = (IApplication*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (!m_pIApplication)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);


	if (m_pIApplication->AppMsgProc(hWnd, uMsg, wParam, lParam))
		return 0;
	/*
	switch (uMsg)
	{
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY:
	g_Application.Stop();
	break;
	case WM_SHOWWINDOW:
	{
	g_Application.ToggleActive(wParam != 0);
	LOGINFO("Window is %s", g_Application.IsActive() ? "shown" : "hidden");

	}
	break;
	case WM_ACTIVATE:
	{
	bool active = g_Application.IsActive();
	if (HIWORD(wParam))
	{
	g_Application.ToggleActive(false);
	}
	else
	{
	WINDOWPLACEMENT lpwndpl;
	lpwndpl.length = sizeof(lpwndpl);
	if (LOWORD(wParam) != WA_INACTIVE)
	{
	if (GetWindowPlacement(hWnd, &lpwndpl))
	g_Application.ToggleActive(lpwndpl.showCmd != SW_HIDE);
	}
	else
	{
	g_Application.ToggleActive(false);
	}

	}
	LOGINFO("Window is %s", g_Application.IsActive() ? "active" : "inactive");
	}
	break;
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
	g_Application.ToggleFocused(uMsg == WM_SETFOCUS);
	g_WindowSystem.NotifyAppFocusChange(g_Application.IsFocused());
	if (uMsg == WM_KILLFOCUS)
	{
	std::string proces;
	if (CUtil::GetFocussedProcess(proces))
	LOGINFO("Focus switched to process %s", proces.c_str());
	}
	break;
	case WM_SYSKEYDOWN:

	switch (wParam)
	{
	case VK_F4: //alt-f4, default event quit.
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	case VK_F5:
	g_GuiSystem.Refresh();
	return(0);
	case VK_F6:

	return(0);
	case VK_RETURN: //alt-return
	if (g_WindowSystem.IsFullScreen() == false) {
	g_WindowSystem.ToggleFullScreen(true);
	}
	else {
	g_WindowSystem.ToggleFullScreen(false);
	}
	return(0);
	}
	break;
	case WM_MOUSELEAVE:
	m_bMouseInWindow = false;
	g_GuiSystem.MouseLeaves();
	break;
	case WM_NCMOUSELEAVE:
	m_bMouseInWindow = false;
	g_GuiSystem.MouseLeaves();
	break;
	case WM_MOUSEMOVE:
	{
	//This hackensures that WM_MOUSELEAVES will be send after mouse leave.
	if (!m_bMouseInWindow) {
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = hWnd;
	TrackMouseEvent(&tme);
	m_bMouseInWindow = true;
	}

	g_WindowSystem.injectMousePosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	break;
	}
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	if (uMsg == WM_LBUTTONDOWN) return g_WindowSystem.injectMouseButtonDown(MouseButton::LeftButton);
	else if (uMsg == WM_MBUTTONDOWN) return g_WindowSystem.injectMouseButtonDown(MouseButton::MiddleButton);
	else if (uMsg == WM_RBUTTONDOWN) return g_WindowSystem.injectMouseButtonDown(MouseButton::RightButton);
	return(0);
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	if (uMsg == WM_LBUTTONUP) return g_WindowSystem.injectMouseButtonUp(MouseButton::LeftButton);
	else if (uMsg == WM_MBUTTONUP) return g_WindowSystem.injectMouseButtonUp(MouseButton::MiddleButton);
	else if (uMsg == WM_RBUTTONUP) return g_WindowSystem.injectMouseButtonUp(MouseButton::RightButton);
	return(0);
	case 0x020A: // WM_MOUSEWHEEL:
	//g_GuiSystem.InjectMouseWheel(static_cast<float>((short)HIWORD(wParam)) / static_cast<float>(120));
	g_WindowSystem.injectMouseWheel(static_cast<float>((short)HIWORD(wParam)) / static_cast<float>(120));
	return(0);
	case WM_SIZE:
	// Save the new client area dimensions.
	w = GET_X_LPARAM(lParam);
	h = GET_Y_LPARAM(lParam);

	if (wParam == SIZE_MINIMIZED) {
	minimized = true;
	maximized = false;
	g_Application.ToggleActive(false);

	}
	else if (wParam == SIZE_MAXIMIZED) {
	minimized = false;
	maximized = true;
	g_Application.ToggleActive(true);

	g_WindowSystem.Resize(w, h);

	}
	else if (wParam == SIZE_RESTORED) {

	// Restoring from minimized state?
	if (minimized){
	g_Application.ToggleActive(true);
	minimized = false;
	}

	// Restoring from maximized state?
	else if (maximized){
	g_Application.ToggleActive(true);
	maximized = false;
	g_WindowSystem.Resize(w, h);

	}

	else if (!resizing) {
	// If user is dragging the resize bars, we do not resize
	// the buffers here because as the user continuously
	// drags the resize bars, a stream of WM_SIZE messages are
	// sent to the window, and it would be pointless (and slow)
	// to resize for each WM_SIZE message received from dragging
	// the resize bars.  So instead, we reset after the user is
	// done resizing the window and releases the resize bars, which
	// sends a WM_EXITSIZEMOVE message.
	}
	else {
	resized = true;
	//eventSized.resize.w = w;
	//eventSized.resize.h = h;
	//m_pEventFunc(eventSized);
	//g_WindowSystem.Resize(w, h);

	}
	}

	return(0);

	case WM_MOVE:
	moved = true;
	rw = GET_X_LPARAM(lParam);
	rh = GET_Y_LPARAM(lParam);
	return(0);

	case WM_ENTERSIZEMOVE:
	resizing = true;
	g_Application.ToggleActive(false);
	return(0);

	case WM_EXITSIZEMOVE:
	resizing = false;

	//Send Event when window has been moved
	if (moved){
	moved = false;
	g_WindowSystem.Move(rw, rh);
	}

	//Send Event when window has been resized
	if (resized) {
	g_WindowSystem.Resize(w, h);
	resized = false;
	}

	g_Application.ToggleActive(true);

	return(0);
	case WM_PAINT:
	{
	HDC         hDC;
	PAINTSTRUCT ps;

	hDC = BeginPaint(hWnd, &ps);
	EndPaint(hWnd, &ps);
	break;
	}

	case WM_KEYDOWN:
	{
	CEGUI::Key::Scan newkeystate = (CEGUI::Key::Scan)MapVirtualKey(wParam, MAPVK_VK_TO_VSC);

	CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyDown((CEGUI::Key::Scan)MapVirtualKey(wParam, MAPVK_VK_TO_VSC));

	break;
	}
	case WM_KEYUP:
	{
	CEGUI::Key::Scan newkeystate = (CEGUI::Key::Scan)MapVirtualKey(wParam, MAPVK_VK_TO_VSC);

	CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyUp((CEGUI::Key::Scan)MapVirtualKey(wParam, MAPVK_VK_TO_VSC));
	break;

	}
	case WM_CHAR:
	CEGUI::System::getSingleton().getDefaultGUIContext().injectChar((CEGUI::utf32) wParam);
	break;

	default:
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));

	break;
	}
	*/
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

