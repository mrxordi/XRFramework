#include "stdafx.h"
#include <vld.h>
#include "TestMFCApp.h"
#include "MainFrame.h"
#include "../XRCommon/Util.h"
#include "../XRCommon/log/Log.h"
#include "../XRCommon/utils/ConverterFactory.h"


CTestMFCApp::CTestMFCApp() : m_mainFrame(NULL)
{
}


CTestMFCApp::~CTestMFCApp()
{
}

BOOL CTestMFCApp::InitInstance()
{
	std::string appPath, tempPath, docPath, homePath;

	appPath = CUtil::ResolveExecutablePath();
	tempPath = CUtil::ResolveSystemTempPath();
	docPath = CUtil::ResolveDocPath();
	homePath = CUtil::ResolveUserPath();


	CSpecialProtocol::SetAPPPath(appPath);
	CSpecialProtocol::SetAPPBinPath(appPath);
	CSpecialProtocol::SetTempPath(tempPath);
	CSpecialProtocol::SetDocPath(docPath);
	CSpecialProtocol::SetHomePath(homePath);

	CLog::Create();

	if (!g_LogPtr->Init(std::string("special://app/log.txt")))
	{
		fprintf(stderr, "Could not init logging classes. Permission errors on (%s)\n",
			CSpecialProtocol::TranslatePath("special://home/").c_str());
	}
	LOGINFO("Start application initialization. Running on thread: %d", GetCurrentThreadId());
	CSpecialProtocol::LogPaths();

	m_settings = std::make_unique<CAppSettings>();
	// Be careful if you move that code: IDR_MAINFRAME icon can only be loaded from the executable,
	// LoadIcon can't be used after the language DLL has been set as the main resource.
	HICON icon = LoadIcon(IDR_MAINFRAME);

	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(WNDCLASS));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hInstance = AfxGetInstanceHandle();
	wndcls.hIcon = icon;
	wndcls.hCursor = LoadCursor(IDC_ARROW);
	wndcls.hbrBackground = 0;//(HBRUSH)(COLOR_WINDOW + 1); // no bkg brush, the view and the bars should always fill the whole client area
	wndcls.lpszMenuName = nullptr;
	wndcls.lpszClassName = MPC_WND_CLASS_NAME;

	if (!AfxRegisterClass(&wndcls)) {
		AfxMessageBox(_T("MainFrm class registration failed!"));
		return FALSE;
	}

	m_mutexOneInstance.Create(nullptr, true, MPC_WND_CLASS_NAME);
//	long long i = GetLastError();
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		DWORD res = WaitForSingleObject(m_mutexOneInstance.m_h, 5000);
		if (res == WAIT_OBJECT_0 || res == WAIT_ABANDONED) {
			HWND hWnd = ::FindWindow(MPC_WND_CLASS_NAME, nullptr);
			if (hWnd) {
				DWORD dwProcessId = 0;
				if (GetWindowThreadProcessId(hWnd, &dwProcessId) && dwProcessId) {
					VERIFY(AllowSetForegroundWindow(dwProcessId));
				}
				else {
					ASSERT(FALSE);
				}
				if (IsIconic(hWnd)) {
					ShowWindow(hWnd, SW_RESTORE);
				}
				m_mutexOneInstance.Close();
				return false;
			}
		}
	}

	if (!__super::InitInstance()) {
		LOGFATAL("Init Instance Failed!");
		return FALSE;
	}
	m_mainFrame = DEBUG_NEW CTestMFCFrame;
	m_pMainWnd = m_mainFrame;

	if (!m_mainFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, nullptr, nullptr)) {
		AfxMessageBox(_T("CMainFrame::LoadFrame failed!"));
		return FALSE;
	}

	m_mainFrame->SetIcon(icon, true);
	m_mainFrame->ShowWindow(SW_SHOW);
	m_mainFrame->UpdateWindow();
	m_mainFrame->SetFocus();

	m_mutexOneInstance.Release();

	return true;
}

int CTestMFCApp::ExitInstance()
{
	ConverterFactory::DestroyAll();
	g_LogPtr->Destroy();
	return __super::ExitInstance();
}

/*
BOOL CTestMFCApp::Run()
{
	ASSERT_VALID(this);

	// for tracking the idle time state
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;
	MSG m_msgCur = { 0 };
	// acquire and dispatch messages until a WM_QUIT message is received.
	for (;;)
	{
		// phase1: check to see if we can do idle work
		while (bIdle &&
			!::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE))
		{
			// call OnIdle while in bIdle state
			if (!OnIdle(lIdleCount++))
				bIdle = FALSE; // assume "no idle" state
		}

		// phase2: pump messages while available
		do
		{
			// pump message, but quit on WM_QUIT
			if (!PumpMessage())
				return ExitInstance();

			// reset "no idle" state after pumping "normal" message
			if (IsIdleMessage(&m_msgCur))
			{
				bIdle = TRUE;
				lIdleCount = 0;
			}

		} while (::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE));

	}

	ASSERT(FALSE);  // not reachable
}*/

BOOL CTestMFCApp::OnIdle(LONG lCount) {
	if (__super::OnIdle(lCount))
		return TRUE;

	m_mainFrame->OnRender();
	return TRUE;
}


CTestMFCApp g_app;

BEGIN_MESSAGE_MAP(CTestMFCApp, CWinApp)
END_MESSAGE_MAP()