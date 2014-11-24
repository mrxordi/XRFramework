#include "stdafx.h"
#include "Application.h"
#include "../XRFramework/log/Log.h"
#include "../XRFramework/window/WindowEvents.h"
#include "../XRFramework/RenderControl.h"
#include "../XRFramework/render/RenderSystemDX.h"
#include "../XRFramework/filesystem/CurlFile.h"
#include "../XRFramework/filesystem/URL.h"


template<> Application* Singleton<Application>::ms_Singleton = 0;

Application::Application() : m_bRunning(false)
{
}


Application::~Application()
{
}

bool Application::Initialize(HINSTANCE hInstance, int nCmdShow) {

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, 100);
	LoadString(hInstance, IDC_XRTESTS, szWindowClass, 100);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		LOGDEBUG("Failed to initialize window system.");
		return false;
	}


	XFILE::CCurlFile file;
	LOGINFO("You are %s internet.", file.IsInternet() ? "connected to" : "disconnected from");
	file.SetUserAgent("XBMC");
	//CURL url("http://weeb.tv/api/setPlayer");
	//url.SetOptions("?channel=0&platform=XBMC");
	//file.Open(url);
	std::string str;
	file.Post("http://weeb.tv/api/setPlayer", "channel=tvp22hd&platform=XBMC&username=xordi@wp.pl&password=8085", str);


	//file.ReadData(str);
	LOGINFO(str.c_str());
	str = CURL::Decode(str);
	LOGINFO(str.c_str());

	return true;
}

void Application::Run() {

	LOGINFO("Running the application...");

	m_bRunning = true;
	while (m_bRunning)
	{
		CWindowEvents::MessagePump();
		g_DXRendererPtr->BeginRender();
		g_DXRendererPtr->EndRender();

	}
	g_DXRendererPtr->DestroyRenderSystem();
}

bool Application::AppMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	PAINTSTRUCT ps;
	HDC hdc;

	switch (uMsg)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_QUIT:
	case WM_DESTROY:
		PostQuitMessage(0);
		g_applicationPtr->Stop();
		break;
	default:
		return false;
	}
	return true;
}

bool Application::OnCreate(HWND hWnd) {

	//handles the WM_CREATE message of the main, parent window; return -1 to fail
	//window creation
	RECT rc = { 20, 20, 400, 250 };
	m_rendercontrol.Initialize(hWnd, hInst, rc);
	cRenderSystemDX::Create();

	g_DXRendererPtr->InitRenderSystem(&m_rendercontrol);


	return true;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM Application::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = CWindowEvents::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_XRTESTS));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_XRTESTS);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL Application::InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hInst = hInstance; // Store instance handle in our global variable

	m_hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, this);

	if (!m_hWnd)
	{
		return FALSE;
	}

	ShowWindow(m_hWnd, nCmdShow);
	UpdateWindow(m_hWnd);

	return TRUE;
}