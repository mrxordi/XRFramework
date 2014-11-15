#pragma once

class IApplication
{
public:
	IApplication(){};
	virtual ~IApplication(){};

	virtual bool AppMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual bool OnCreate(HWND hWnd) = 0;
};