#include <stdafxf.h>
#include "MainFrame.h"
#include "main.h"
#include "wxDX10renderer.h"
#include "utils/XRect.h"

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(ID_Quit, MyFrame::OnQuit)
EVT_MENU(ID_About, MyFrame::OnAbout)
EVT_MENU(ID_ResizeWideo, MyFrame::OnResizeWideo)
EVT_MENU_RANGE(ID_Debug_ffmpeg, ID_Debug_curl, MyFrame::DebugToggle)
EVT_CLOSE(MyFrame::OnCloseWindow)
END_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
: wxFrame((wxFrame *)NULL, -1, title, pos, size)
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

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuDebug, "&Debug Options");

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Welcome to wxWindows!");

	wxDX10renderer* widget = new wxDX10renderer(this, -1);
	wxGetApp().m_VideoRenderer->Configure(1280, 534, 24.0, CONF_FLAGS_YUVCOEF_BT601, RENDER_FMT_YUV420P, widget);
}


void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(TRUE);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{

	wxMessageBox("This is a wxWindows Hello world sample",
		"About Hello World", wxOK | wxICON_INFORMATION, this);
}

void MyFrame::OnResizeWideo(wxCommandEvent& WXUNUSED(event)) 
{
	wxGetApp().m_VideoRenderer->Configure(640, 480, 24.0f, CONF_FLAGS_YUVCOEF_BT601, ERenderFormat::RENDER_FMT_NV12, NULL);
}

void MyFrame::OnCloseWindow(wxCloseEvent& event)
{
	wxSize size = GetSize();
	CMonitor* monitor = wxGetApp().m_monitors->GetNearestMonitor(GetHWND());

	if (monitor)
		GetAppSettings().SaveCurrentRect(XRect(0, 0, size.x, size.y), monitor->GetOrdinal());

	wxVideoRendererEvent* ev = new wxVideoRendererEvent(wxGetApp().m_VideoRenderer.get(), wxVideoRendererEvent::VR_ACTION_DETACH);
	QueueEvent(ev);

	wxFrame::OnCloseWindow(event);
}

void MyFrame::DebugToggle(wxCommandEvent& event)
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

	}
}
