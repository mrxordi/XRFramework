#include <stdafxf.h>
#include "Context.h"
#include "MainFrame.h"
#include "VideoBox.h"
#include "main.h"
#include "wxDX10Display.h"
#include "utils/XRect.h"
#include "XRCommon/log/Log.h"


BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_Quit, MainFrame::OnQuit)
EVT_MENU(ID_About, MainFrame::OnAbout)
EVT_MENU(ID_ResizeWideo, MainFrame::OnResizeWideo)
EVT_MENU_RANGE(ID_Debug_ffmpeg, ID_Debug_curl, MainFrame::DebugToggle)
EVT_CLOSE(MainFrame::OnCloseWindow)
// EVT_BUTTON(IDB_Seek, MainFrame::OnSeekButton)
// EVT_BUTTON(IDB_Read, MainFrame::OnReadPacket)
END_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title, const wxRect& rect)
	: wxFrame((wxFrame *)NULL, -1, title, wxDefaultPosition, rect.GetSize(), wxDEFAULT_FRAME_STYLE| wxCLIP_CHILDREN), 
	m_context(std::make_unique<Context>())
{
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(ID_About, "&About...");
	menuFile->Append(ID_ResizeWideo, "&Resize Wideo...");
	menuFile->AppendSeparator();
	menuFile->Append(ID_Quit, "E&xit");

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

	CreateStatusBar();
	SetStatusText("Welcome to wxWindows!");
	m_context->frame = this;
	m_context->parent = this;
	
	wxPanel *panel = new wxPanel(this, wxID_ANY);
	wxPanel *videobox = new VideoBox(this, m_context.get());
	wxDX10Display* widget = new wxDX10Display(panel, m_context.get(), -1);

	m_sizer = new wxBoxSizer(wxHORIZONTAL);
	m_sizer->Add(widget, wxSizerFlags(1).Expand().Border());
	panel->SetSizer(m_sizer);

	wxSizer *sizerTop = new wxBoxSizer(wxVERTICAL);
	sizerTop->Add(panel, wxSizerFlags(1).Expand().Border());
	sizerTop->Add(videobox, wxSizerFlags(0).Border().Expand());

	SetSizer(sizerTop);

	//wxGetApp().m_VideoRenderer->Configure(1280, 534, 24.0, CONF_FLAGS_YUVCOEF_BT601, RENDER_FMT_YUV420P, widget);
	
	// create the controls
}


void MainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(TRUE);
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{

	wxMessageBox("This is a wxWindows Hello world sample",
		"About Hello World", wxOK | wxICON_INFORMATION, this);
}

void MainFrame::OnResizeWideo(wxCommandEvent& WXUNUSED(event)) 
{

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
