#include <stdafxf.h>
#include "MainFrame.h"
#include "main.h"
#include "wxDX10renderer.h"
#include "utils/XRect.h"

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(ID_Quit, MyFrame::OnQuit)
EVT_MENU(ID_About, MyFrame::OnAbout)
EVT_CLOSE(MyFrame::OnCloseWindow)
END_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
: wxFrame((wxFrame *)NULL, -1, title, pos, size)
{
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(ID_About, "&About...");
	menuFile->AppendSeparator();
	menuFile->Append(ID_Quit, "E&xit");

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Welcome to wxWindows!");

	wxDX10renderer* widget = new wxDX10renderer(this, -1);
	//wxGetApp().m_VideoRenderer->Configure(1280, 534, 24.0, CONF_FLAGS_YUVCOEF_BT601, RENDER_FMT_YUV420P, widget);
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

void MyFrame::OnCloseWindow(wxCloseEvent& event)
{
	wxSize size = GetSize();
	CMonitor* monitor = wxGetApp().m_monitors->GetNearestMonitor(GetHWND());
	if (monitor) {
		GetAppSettings().SaveCurrentRect(XRect(0, 0, size.x, size.y), monitor->GetOrdinal());
	}

	wxFrame::OnCloseWindow(event);
}
