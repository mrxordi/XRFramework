#pragma once
#include "wx/frame.h"
#include "wx/window.h"
#include "wx/sizer.h"
#include "wx/textctrl.h"
#include <memory>
#include "XRFramework/utils/JobManager.h"
#include "XRFramework/filesystem/FileItem.h"
#include "XRFramework/filesystem/CurlGlobal.h"
#include "XRFramework/filesystem/FileOperationJob.h"


class MyFrame : public wxFrame, public IJobCallback
{
public:

	MyFrame(const wxString& title,
		const wxPoint& pos, const wxSize& size);

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnResizeWideo(wxCommandEvent& event);
	void DebugToggle(wxCommandEvent& event);

	void OnCloseWindow(wxCloseEvent& event);
	void OnAddSession(wxCommandEvent& event);
	void OnRunSession(wxCommandEvent& event);
	void OnCheckIdle(wxCommandEvent& event);
	void OnDeleteAll(wxCommandEvent& event);

	wxFlexGridSizer * m_sizer;

	wxTextCtrl * m_text;

	CURL_HANDLE* m_handle;
	
	DECLARE_EVENT_TABLE()

	virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job) override;

	virtual void OnJobProgress(unsigned int jobID, unsigned int progress, unsigned int total, const CJob *job) override;

	//FileOpsTests testy;

	FileOperationJob* m_job;
	CFileItemList m_list;

};

enum
{
	ID_Quit = 1,
	ID_ResizeWideo,
	ID_About,
	ID_Debug_ffmpeg,
	ID_Debug_curl
};

enum
{
	ID_CheckIdle = 20,
	ID_Text,
	ID_AddCurlSession,
	ID_RunCurlSession,
	ID_DeleteAll

};