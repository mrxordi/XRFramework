#include "stdafxf.h"
#include "StatusBar.h"


StatusBar::StatusBar()
{
}


StatusBar::~StatusBar()
{
}

bool StatusBar::Initialize() {
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_BAR_CLASSES; // toolbary, statusbary i tooltipy
	InitCommonControlsEx(&icc);
	return true;
}

void StatusBar::OnCreate(HWND parent, HINSTANCE hInstance) {
	if (m_hwnd)
		LOGERR("StatusBar already created.")
	m_hwnd = CreateWindowEx(0, STATUSCLASSNAME, NULL, SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE
		, 0, 0, 0, 0, parent, (HMENU)200, hInstance, NULL);
	int iStatusBarWidths[] = { 200, 300 };
	SendMessage(m_hwnd, SB_SETPARTS, 2, (LPARAM)iStatusBarWidths);
	SetText();
}

void StatusBar::OnDestory() {

}

void StatusBar::OnResize(int x, int y) {
	int p1 = x*0.8f;
	int iStatusBarWidths[] = { p1, -1 };
	SendMessage(m_hwnd, SB_SETPARTS, 2, (LPARAM)iStatusBarWidths);
	SendMessage(m_hwnd, WM_SIZE, 0, 0);
}

void StatusBar::SetText(std::string text) {
	if (text.empty())
		SendMessage(m_hwnd, SB_SETTEXT, 1, (LPARAM) "Application started.");

	SendMessage(m_hwnd, SB_SETTEXT, 1, (LPARAM) text.c_str());

}
