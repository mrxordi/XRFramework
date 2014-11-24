// XRTestMDI.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "XRTestMDI.h"

#include "Application.h"
#include "../XRFramework/filesystem/SpecialProtocol.h"
#include "../XRFramework/log/Log.h"
#include "../XRFramework/Util.h"

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);



	CStdString appPath, tempPath, docPath, homePath;

	appPath = CUtil::ResolveExecutablePath();
	tempPath = CUtil::ResolveSystemTempPath();
	docPath = CUtil::ResolveDocPath();
	homePath = CUtil::ResolveUserPath();

	CSpecialProtocol::SetAPPPath(appPath);
	CSpecialProtocol::SetAPPBinPath(appPath);
	CSpecialProtocol::SetTempPath(tempPath);
	CSpecialProtocol::SetDocPath(docPath);
	CSpecialProtocol::SetHomePath(homePath);


	DWORD m_threadid = GetCurrentThreadId();

	if (!Logger::Init("special://app/log.txt"))
	{
		fprintf(stderr, "Could not init logging classes. Permission errors on (%s)\n",
			CSpecialProtocol::TranslatePath("special://home/").c_str());
		return false;
	}

	LOGDEBUG("cApplication Initialization. App Running on thread: %d", m_threadid);

	CSpecialProtocol::LogPaths();

	Application app;
	g_applicationPtr->Initialize(hInstance, nCmdShow);
	g_applicationPtr->Run();


	return (int)0;