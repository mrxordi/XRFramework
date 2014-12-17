#include "stdafx.h"
#include <vld.h>

#include "Application.h"
#include "CurlJsonCConverterLoggerT.h"
#include "utils\JobManager.h"
//#include "../XRFramework/log/Logger.h"
//#include "../XRFramework/log/LoggerDAL.h"
//#include "../XRFramework/log/LogDefines.h"
#include "../XRFramework/window/WindowEvents.h"
#include "../XRFramework/window/RenderControl.h"
#include "../XRFramework/render/RenderSystemDX.h"
#include "../XRFramework/core/VideoRenderers/WinRenderer.h"
#include "../XRFramework/core/VideoRenderers/YUV2RGBShader.h"



template<> Application* Singleton<Application>::ms_Singleton = 0;

Application::Application() : m_bRunning(false), IApplication(this)
{
}


Application::~Application()
{
	g_DXRendererPtr->Destroy();
}

bool Application::Initialize(HINSTANCE hInstance, int nCmdShow) {

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, 100);
	LoadString(hInstance, IDC_XRTESTS, szWindowClass, 100);
	MyRegisterClass(hInstance);

	cRenderSystemDX::Create();

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		LOGDEBUG("Failed to initialize window system.");
		return false;
	}

	//CurlJsonCharsetConverterLoggerTests();

	return true;
}

void Application::Run() {

	LOGINFO("Running the application...");

	m_bRunning = true;
	while (m_bRunning)
	{
		if (CWindowEvents::GetQueueSize())
			CWindowEvents::MessagePump();

		if (g_DXRendererPtr->BeginRender()) {
			//m_testshader.Render();
			m_yuvshader.Render(XRect(5, 5, 635, 475), XRect(10, 10, 770, 570), 50.0f, 50.0f, CONF_FLAGS_YUVCOEF_BT601, &m_yuvbuffer);
		}
		g_DXRendererPtr->PresentRender();
		g_DXRendererPtr->EndRender();

	}
	OnDestroy();
}

bool Application::AppMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	PAINTSTRUCT ps;
	HDC hdc;
	int x, y;

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
	case WM_SIZE:
		// Save the new client area dimensions.
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);

		m_Statusbar->OnResize(x, y);

	default:
		return false;
	}
	return true;
}

bool Application::OnCreate(HWND hWnd) {

	//handles the WM_CREATE message of the main, parent window; return -1 to fail
	//window creation
	m_Statusbar->OnCreate(hWnd, hInst);

	RECT rc = { 20, 20, 800, 600 };

	m_rendercontrol.Initialize(hWnd, hInst, rc);
	g_DXRendererPtr->InitRenderSystem(&m_rendercontrol);
	m_testshader.Create();


	m_yuvshader.Create(640, 480, RENDER_FMT_YUV420P);

	m_yuvbuffer.Create(RENDER_FMT_YUV420P, 640, 480);
	m_yuvbuffer.StartDecode();
	//m_yuvbuffer.Clear();

	UINT dstPitch[3], dstPitchBefore[3];
	unsigned char* dstData[3], *dstDataBefore[3];

	for (UINT i = 0; i < 3; i++) {
		dstPitchBefore[i] = dstPitch[i] = m_yuvbuffer.planes[i].rect.RowPitch;
		dstDataBefore[i] = dstData[i] = (unsigned char*)m_yuvbuffer.planes[i].rect.pData;
	}

	File fe;
	auto_buffer buff;
	const char* filelist[3] = { "special://app/system/frame.yyy", "special://app/system/frame.uuu", "special://app/system/frame.vvv" };
	YV12Image image;
	{
		fe.LoadFile(filelist[0], buff);
		uint8_t* dataY = new uint8_t[buff.size()];
		memcpy(dataY, buff.get(), buff.size());
		image.plane[0] = dataY;
		image.planesize[0] = buff.size();
		image.stride[0] = 640;

		fe.LoadFile(filelist[1], buff);
		uint8_t* dataU = new uint8_t[buff.size()];
		memcpy(dataU, buff.get(), buff.size());
		image.plane[1] = dataU;
		image.planesize[1] = buff.size();
		image.stride[1] = 320;


		fe.LoadFile(filelist[2], buff);
		uint8_t* dataV = new uint8_t[buff.size()];
		memcpy(dataV, buff.get(), buff.size());
		image.plane[2] = dataV;
		image.planesize[2] = buff.size();
		image.stride[2] = 320;

		image.width = 640;
		image.height = 480;
		image.cshift_x = 1;
		image.cshift_y = 1;
	}

	uint8_t* s = image.plane[0];
	uint8_t* d = (BYTE*)m_yuvbuffer.planes[0].rect.pData;
	int w = m_yuvbuffer.planes[0].texture.GetWidth();
	int h = m_yuvbuffer.planes[0].texture.GetHeight();

	for (int i = 0; i < h; i++) {
		memcpy(d, s, w);
		s += image.stride[0];
		d += m_yuvbuffer.planes[0].rect.RowPitch;
	}

	s = image.plane[1];
	d = (BYTE*)m_yuvbuffer.planes[1].rect.pData;
	w = (m_yuvbuffer.planes[1].texture.GetWidth() >> 1);
	h = (m_yuvbuffer.planes[1].texture.GetHeight() >> 1);

	for (int i = 0; i < h; i++) {
		memcpy(d, s, w);
		s += image.stride[1];
		d += m_yuvbuffer.planes[1].rect.RowPitch;
	}

	s = image.plane[2];
	d = (BYTE*)m_yuvbuffer.planes[2].rect.pData;

	for (int i = 0; i < h; i++) {
		memcpy(d, s, w);
		s += image.stride[2];
		d += m_yuvbuffer.planes[2].rect.RowPitch;
	}


	m_yuvbuffer.StartRender();

	delete image.plane[0];
	delete image.plane[1];
	delete image.plane[2];


	return true;
}

bool Application::OnDestroy() {
	CJobManager::GetInstance().CancelJobs();
	//m_yuvbuffer.Release();
	g_DXRendererPtr->DestroyRenderSystem();
	m_Statusbar->OnDestory();
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