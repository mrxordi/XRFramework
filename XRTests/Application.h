#pragma once
#include "../XRFramework/utils/Singleton.h"
#include "../XRFramework/IApplication.h"
#include "../XRFramework/window/RenderControl.h"
#include "../XRFramework/filesystem/File.h"
#include "../XRFramework/window/StatusBar.h"
#include "../XRFramework/core/VideoRenderers/TestShader.h"
#include "../XRFramework/core/VideoRenderers/YUV2RGBShader.h"
#include <string>

#include "Resource.h"

class Application : public Singleton<Application>, public IApplication
{
public:
	Application();
	virtual ~Application();

	bool Initialize(HINSTANCE hInstance, int nCmdShow);
	void Run();
	void Stop() { m_bRunning = false; }

	bool AppMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool OnCreate(HWND hWnd);
	bool OnDestroy();

	static Application& Create() { return *new Application; }
	static void	Destroy() { delete Application::getSingletonPtr(); }


private:
	// Forward declarations of functions included in this code module:
	ATOM				MyRegisterClass(HINSTANCE hInstance);
	BOOL				InitInstance(HINSTANCE, int);

private:
	bool m_bRunning;
	// Global Variables:
	HINSTANCE hInst;								// current instance
	TCHAR szTitle[100];					// The title bar text
	TCHAR szWindowClass[100];			// the main window class name
	HWND m_hWnd;
	RenderControl m_rendercontrol;
	TestShader m_testshader;
	YUV2RGBShader m_yuvshader;
	YUVBuffer m_yuvbuffer;

};

#define g_applicationPtr Application::getSingletonPtr()
#define g_application Application::getSingleton()