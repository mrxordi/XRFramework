#pragma once
#include "wx/frame.h"
#include "wx/window.h"
#include "wx/sizer.h"
#include "wx/textctrl.h"
#include <memory>
#include "FileOpsTests.h"
#include "XRFramework/filesystem/CurlGlobal.h"


class MyFrame : public wxFrame
{
public:

	MyFrame(const wxString& title,
		const wxPoint& pos, const wxSize& size);

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnResizeWideo(wxCommandEvent& event);
	void DebugToggle(wxCommandEvent& event);

	void OnCloseWindow(wxCloseEvent& event);
	void OnCreateSession(wxCommandEvent& event);
	void OnCheckIdle(wxCommandEvent& event);
	void OnDeleteAll(wxCommandEvent& event);

	wxFlexGridSizer * m_sizer;

	wxTextCtrl * m_text;

	CURL_HANDLE* m_handle;
	
	DECLARE_EVENT_TABLE()

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
	ID_CreateCurlSession,
	ID_DeleteAll

};