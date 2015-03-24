#include <stdafxf.h>
#include "MainFrame.h"
#include "main.h"
#include "utils/XRect.h"

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(ID_Quit, MyFrame::OnQuit)
EVT_MENU(ID_About, MyFrame::OnAbout)
EVT_MENU_RANGE(ID_Debug_ffmpeg, ID_Debug_curl, MyFrame::DebugToggle)
EVT_CLOSE(MyFrame::OnCloseWindow)
EVT_BUTTON(ID_AddCurlSession, MyFrame::OnAddSession)
EVT_BUTTON(ID_RunCurlSession, MyFrame::OnRunSession)
EVT_BUTTON(ID_CheckIdle, MyFrame::OnCheckIdle)
EVT_BUTTON(ID_DeleteAll, MyFrame::OnDeleteAll)
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

	//wxPanel *panel = new wxPanel(this, -1);
	wxBoxSizer *hbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *vbox = new wxBoxSizer(wxHORIZONTAL);
	m_sizer = new wxFlexGridSizer(2, 2, 9, 25);
	

	wxButton* button = new wxButton(this, ID_AddCurlSession, "Add Session");
	m_sizer->Add(button);

	button = new wxButton(this, ID_RunCurlSession, "Run Session");
	m_text = new wxTextCtrl(this, ID_Text, wxEmptyString, wxDefaultPosition, wxSize(300, 25));

	m_sizer->Add(button);
	m_sizer->Add(m_text, 1, wxEXPAND);


	hbox->Add(m_sizer, 1, wxALL | wxEXPAND, 15);
	button = new wxButton(this, ID_CheckIdle, "Check idle");
	vbox->Add(button, 0, wxCENTER, 4);
	button = new wxButton(this, ID_DeleteAll, "Delete all");
	vbox->Add(button, 0, wxCENTER, 4);
	hbox->Add(vbox, 1);
	SetSizer(hbox);
	Centre();
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

void MyFrame::OnAddSession(wxCommandEvent& event)
{
	std::string aaa = m_text->GetLineText(0);

	m_list.Add(CFileItemPtr(new CFileItem(aaa, false)));
}

void MyFrame::OnRunSession(wxCommandEvent& event)
{
	m_job = new FileOperationJob;
	m_job->SetFileOperation(FileOperationJob::ActionCopy, m_list, "special://app/");
	CJobManager::GetInstance().AddJob(m_job, this);
}

void MyFrame::OnCheckIdle(wxCommandEvent& event)
{
	CCurlGlobal::CheckIdle();
}

void MyFrame::OnDeleteAll(wxCommandEvent& event)
{
	CCurlGlobal::UnloadAll();
}

void MyFrame::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
	LOGINFO("Job completed");
}

void MyFrame::OnJobProgress(unsigned int jobID, unsigned int progress, unsigned int total, const CJob *job)
{
	CONST FileOperationJob* fjob = static_cast<CONST FileOperationJob*> (job);
	LOGINFO("Job ID:%u Progress:%u Total:%u", jobID, progress, total);
	LOGINFO("File:%s Averange speed:%s Operation:%s", fjob->GetCurrentFile().c_str(), fjob->GetAverageSpeed().c_str(), fjob->GetCurrentOperation().c_str());

}

