#pragma once
#include "wx/frame.h"
#include <memory>
#include "FileOpsTests.h"


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

	FileOpsTests m_fileopstest;
	
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