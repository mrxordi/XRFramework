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
#include "../XRFramework/system/SysInfo.h"

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
	CSpecialProtocol::InitializePaths();

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

	m_weeb = std::make_unique<CWeebTv>();

	m_bufStream = std::make_unique<CBufferedStream>();
	//m_bufStream->Open(m_weeb->GetStreamLink("tvvvvtvn", false).c_str(), "");
	CFile file("special://temp/filecache002.cache");
	auto_buffer buffeer;
	file.Open();
	file.LoadFile(buffeer);

	uint8_t* pb = (uint8_t*)buffeer.get();
	for (size_t i = 0; i < buffeer.size(); i++)
	{
		if (pb[0] == 0x09 && ((pb[11] & 0xF0) >> 4) == 1 && pb[10] == 0x00 && pb[9] == 0x00 && pb[8] == 0x00) {
			uint32_t ts = AMF_DecodeInt24((char*)(pb + 4));
			ts |= (pb[7] << 24);
			LOGINFO("I have video keyframe, timestamp: %u, at cache offset %u.", ts, i);
		}
		pb++;
	}






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
