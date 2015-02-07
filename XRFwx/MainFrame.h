#pragma once
#include "wx/frame.h"


class MyFrame : public wxFrame
{
public:

	MyFrame(const wxString& title,
		const wxPoint& pos, const wxSize& size);

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnCloseWindow(wxCloseEvent& event);
	
	DECLARE_EVENT_TABLE()
};

enum
{
	ID_Quit = 1,
	ID_About,
};