// XRTests.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "XRTests.h"
#include "Application.h"
#include "LoggerContext.h"
#include "DLLoader/LoaderFactory.h"
#include "filesystem/DllLibCurl.h"
#include "../XRFramework/filesystem/SpecialProtocol.h"
#include "../XRFramework/log/Log.h"
#include "../XRFramework/Util.h"
#include "../XRFramework/utils/ConverterFactory.h"

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	Context context;

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

	CLog::Create();

	if (!g_LogPtr->Init(std::string("special://app/log.txt")))
	{
		fprintf(stderr, "Could not init logging classes. Permission errors on (%s)\n",
			CSpecialProtocol::TranslatePath("special://home/").c_str());
		return false;
	}
	//g_LogPtr->SetExtraLogLevels(LOGCURL);

	LOGINFO("Start application initialization. Running on thread: %d", m_threadid);

	CSpecialProtocol::LogPaths();

	Application::Create();

	g_applicationPtr->Initialize(hInstance, nCmdShow);
	g_applicationPtr->Run();

	Application::Destroy();

	DllLibCurlGlobal::Get().UnloadAll();


	CLog::Destroy();
	ConverterFactory::DestroyAll();


	return (int)0;
}