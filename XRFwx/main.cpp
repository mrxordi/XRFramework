#include <stdafxf.h>
#include <vld.h>
#include "main.h"
#include "MainFrame.h"
#include "../XRCommon/utils/ConverterFactory.h"
#include "../XRCommon/Util.h"
#include "../XRFramework/utils/XRect.h"
#include "../XRCommon/utils/SpecialProtocol.h"
#include "../XRCommon/log/Log.h"
#include "../XRCommon/settings/AppSettings.h"
#include "../XRFramework/filesystem/CurlFile.h"

IMPLEMENT_APP(MyApp)

MyApp::~MyApp()
{
	m_settings->SetInt("system.extralogging", CLog::getSingletonPtr()->GetExtraLogLevel());
	m_settings->SaveSettings();
	m_VideoRenderer.reset();
	ConverterFactory::DestroyAll();
	CLog::Destroy();
}

MyApp::MyApp()
{
	CLog::Create();
	m_threadId = ::GetCurrentThreadId();
}


bool MyApp::OnInit()
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

	if (!g_LogPtr->Init(std::string("special://app/log.txt"))) 
	{
		fprintf(stderr, "Could not init logging classes. Permission errors on (%s)\n",
			CSpecialProtocol::TranslatePath("special://home/").c_str());
	}
	m_settings = std::make_unique<CAppSettings>();
	m_settings->Initialize(std::string("special://app/data/settings.xml"));
	g_LogPtr->SetExtraLogLevels(m_settings->GetInt("system.extralogging"));

	LOGINFO("Start application initialization. Running on thread: %d", GetCurrentThreadId());
	CSpecialProtocol::LogPaths();
	//CCurlFile file;
	//LOGINFO("You are %s internet", file.IsInternet() ? "connected to" : "disconnected from");

	m_monitors = std::make_unique<CMonitors>();
	m_VideoRenderer = std::make_unique<WinRenderer>();

	XRect rect;
	rect.top = 0;
	rect.left = 0;
	rect.bottom = GetAppSettings().GetInt("window.height");
	rect.right = GetAppSettings().GetInt("window.width");
	int monitor = GetAppSettings().GetInt("window.monitor");

	m_monitors->GetMonitor(monitor)->CenterRectToMonitor(rect);

	m_mainFrame = new MyFrame("XRFramework...", rect.TopLeft(), rect.Size());
	m_mainFrame->SetIcon(wxICON(APP_ICON));
	m_mainFrame->Show(TRUE);
	SetTopWindow(m_mainFrame);

	return TRUE;
}

int MyApp::OnExit()
{
	return wxApp::OnExit();
}

bool MyApp::IsCurrentThread() const
{
	return (::GetCurrentThreadId() == m_threadId);
}
