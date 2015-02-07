#pragma once
#include "afxwin.h"
#include <atlsync.h>
#include <memory>
#include "../XRCommon/settings/helpers/Monitors.h"
#include "../XRCommon/utils/SpecialProtocol.h"
#include "../XRCommon/settings/AppSettings.h"

class CTestMFCFrame;

#define MPC_WND_CLASS_NAME L"TestMFCApplicationW"

class CTestMFCApp : public CWinApp
{
public:
	CTestMFCApp();
	~CTestMFCApp();

	virtual BOOL InitInstance() override;
	virtual int ExitInstance() override;

	//virtual BOOL Run() override;
	virtual BOOL OnIdle(LONG lCount) override;


	std::unique_ptr<CAppSettings> m_settings;
	std::unique_ptr<CMonitors> m_monitors;

	DECLARE_MESSAGE_MAP()

private:
	ATL::CMutex m_mutexOneInstance;
	CTestMFCFrame* m_mainFrame;
};

#define AfxGetAppSettings() (*static_cast<CTestMFCApp*>(AfxGetApp())->m_settings.get())
#define AfxGetMyApp()       static_cast<CTestMFCApp*>(AfxGetApp())
#define AfxGetMainFrame()   dynamic_cast<CTestMFCFrame*>(AfxGetMainWnd())