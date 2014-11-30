#pragma once
#include <commctrl.h>
#include <windows.h>
#include "../XRThreads/CriticalSection.h"
#include <string>


class StatusBar : public CCriticalSection
{
public:
	StatusBar();
	~StatusBar();
	bool Initialize();

	void OnCreate(HWND parent, HINSTANCE hInstance);
	void OnDestory();
	void OnResize(int x, int y);

	void SetText(std::string text = "");
	HWND GetHwnd() { return m_hwnd; }
private:
	HWND m_hwnd;
};

