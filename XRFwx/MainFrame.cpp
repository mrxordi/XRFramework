#include <stdafxf.h>
#include "wx/wx.h"
#include "Context.h"
#include "MainFrame.h"
#include "VideoBoxToolbar.h"
#include "main.h"
#include "wxDX10Display.h"
#include "utils/XRect.h"
#include "XRCommon/log/Log.h"
#include "XRCommon/utils/StringUtils.h"
#include "XRFramework/dvdplayer/Player.h"
#include "gui/GUIManager.h"
#include "gui/GUITexture.h"


BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_Quit, MainFrame::OnQuit)
EVT_MENU(ID_About, MainFrame::OnAbout)
EVT_MENU(ID_ResizeWideo, MainFrame::OnResizeWideo)
EVT_MENU_RANGE(ID_Debug_ffmpeg, ID_Debug_curl, MainFrame::DebugToggle)
EVT_CLOSE(MainFrame::OnCloseWindow)
END_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title, const wxRect& rect)
	: wxFrame((wxFrame *)NULL, -1, title, wxDefaultPosition, rect.GetSize(), wxDEFAULT_FRAME_STYLE| wxCLIP_CHILDREN), 
   m_context(std::make_unique<Context>())
{
   //Menu File
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(ID_About, "&About...");
	menuFile->Append(ID_ResizeWideo, "&Resize Wideo...");
	menuFile->AppendSeparator();
	menuFile->Append(ID_Quit, "E&xit");

   //Debug menu addition
	wxMenu* menuDebug = new wxMenu;
	wxMenuItem* item = menuDebug->Append(ID_Debug_curl, "Debug CURL", wxEmptyString, wxITEM_CHECK);
	if (CLog::getSingletonPtr()->IsLogExtraLogged(LOGCURL))
		item->Check(true);

	item = menuDebug->Append(ID_Debug_ffmpeg, "Debug FFMPEG", wxEmptyString, wxITEM_CHECK);
	if (CLog::getSingletonPtr()->IsLogExtraLogged(LOGFFMPEG))
		item->Check(true);

	item = menuDebug->Append(ID_Debug_rtmp, "Debug RTMP", wxEmptyString, wxITEM_CHECK);
	if (CLog::getSingletonPtr()->IsLogExtraLogged(LOGRTMP))
		item->Check(true);

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuDebug, "&Debug Options");

	SetMenuBar(menuBar);
   wxStatusBar* statusbar = CreateStatusBar();
   
	m_context->frame = this;
	m_context->parent = this;
	
	wxPanel *panel = new wxPanel(this, wxID_ANY);
	wxPanel *videoboxToolbar = new VideoBoxToolbar(this, m_context.get());
	wxDX10Display* widget = new wxDX10Display(panel, m_context.get(), -1);

	m_sizer = new wxBoxSizer(wxHORIZONTAL);
	m_sizer->Add(widget, wxSizerFlags(1).Expand().Border());
	panel->SetSizer(m_sizer);

	wxSizer *sizerTop = new wxBoxSizer(wxVERTICAL);
	sizerTop->Add(panel, wxSizerFlags(1).Expand().Border());
	sizerTop->Add(videoboxToolbar, wxSizerFlags(0).Border().Expand());

   m_player = new CPlayer(m_context.get(), nullptr);
    SetSizer(sizerTop);
}

MainFrame::~MainFrame()
{
   DestroyChildren();
   SAFE_DELETE(m_player);
}

void MainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
   SAFE_DELETE(m_player);
	Close(TRUE);
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{

	wxMessageBox("This is a wxWindows Hello world sample",
		"About Hello World", wxOK | wxICON_INFORMATION, this);
//    const CWeebTv::channel* toOpen = wxGetApp().m_weeb->GetChannel("tvvvvtvn");
//    m_player->Open(toOpen);
   CGUITextureBase * texturebase = m_context->videoDisplay->GetGUIManager()->GetTextureManager()->CreateTexture("special://app/data/media");
   texturebase->LoadFromFile("special://app/data/media/img.png");
}

void MainFrame::OnResizeWideo(wxCommandEvent& WXUNUSED(event)) 
{
   static uint32_t a = 0;
   wxProgressEvent* ev = new wxProgressEvent(StringUtils::Format("Postep: %u", a++), this);
   QueueEvent(ev);
}

void MainFrame::OnCloseWindow(wxCloseEvent& event)
{
	wxSize size = GetSize();
	CMonitor* monitor = m_context->monitors->GetNearestMonitor(GetHWND());

	if (monitor)
		GetAppSettings().SaveCurrentRect(XRect(0, 0, size.x, size.y), monitor->GetOrdinal());


	wxFrame::OnCloseWindow(event);
}

void MainFrame::DebugToggle(wxCommandEvent& event)
{
	int extraLogLevel = CLog::getSingletonPtr()->GetExtraLogLevel();

	switch (event.GetId()) 
	{
	case ID_Debug_curl:
		if (event.IsChecked())
			CLog::getSingletonPtr()->SetExtraLogLevels(extraLogLevel | LOGCURL);
		else
			CLog::getSingletonPtr()->SetExtraLogLevels(extraLogLevel & ~(LOGCURL));
		break;
	case ID_Debug_ffmpeg:
		if (event.IsChecked())
			CLog::getSingletonPtr()->SetExtraLogLevels(extraLogLevel | LOGFFMPEG);
		else
			CLog::getSingletonPtr()->SetExtraLogLevels(extraLogLevel & ~(LOGFFMPEG));
		break;
	case ID_Debug_rtmp:
		if (event.IsChecked())
			CLog::getSingletonPtr()->SetExtraLogLevels(extraLogLevel | LOGRTMP);
		else
			CLog::getSingletonPtr()->SetExtraLogLevels(extraLogLevel & ~(LOGRTMP));
		break;

	}
}

wxStatusBar* MainFrame::OnCreateStatusBar(int number /*= 1*/, long style /*= wxSTB_DEFAULT_STYLE*/, wxWindowID id /*= 0*/, const wxString& name /*= wxStatusLineNameStr*/)
{
   wxStatusBar *statusBar = new wxXRStatusBar(this, id, style, name);

   return statusBar;
}

wxXRStatusBar* MainFrame::GetBusyBar()
{
   wxXRStatusBar* ret = dynamic_cast<wxXRStatusBar*>(GetStatusBar());
   return ret;
}
