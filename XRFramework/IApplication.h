#pragma once
#include "window/StatusBar.h"
#include <boost/shared_ptr.hpp>

typedef boost::shared_ptr<StatusBar> StatusBarPtr;

class IApplication
{
public:

	IApplication(IApplication* pThis) : m_Statusbar(new StatusBar) { 
		m_ithis = pThis;
		m_currentThreadID = ::GetCurrentThreadId(); 
	};

	virtual ~IApplication(){};

	virtual bool AppMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual bool OnCreate(HWND hWnd) = 0;

	StatusBarPtr GetStatusBar() { return m_Statusbar; }

	bool IsCurrentThreadId() {
		return (::GetCurrentThreadId() == m_currentThreadID);
	}
	static IApplication* GetIApplicationPtr() { return m_ithis; }
protected:
	StatusBarPtr m_Statusbar;
	DWORD m_currentThreadID;
private:
	static IApplication* m_ithis;
};

#define g_iApplication IApplication::GetIApplicationPtr()