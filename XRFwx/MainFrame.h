#pragma once
#include "wx/wx.h" 
#include "wx/frame.h"
#include "wx/spinctrl.h"
#include <memory>
#include <stdint.h>

struct Context;

class MainFrame : public wxFrame
{
public:

	MainFrame(const wxString& title, const wxRect& rect);

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnResizeWideo(wxCommandEvent& event);
	void DebugToggle(wxCommandEvent& event);

	void OnCloseWindow(wxCloseEvent& event);

// 	void OnReadPacket(wxCommandEvent& event);
// 	void OnSeekButton(wxCommandEvent& event);
	
	DECLARE_EVENT_TABLE()

	wxSizer* m_sizer;
	wxSpinCtrl* m_msInput;
private:
	std::unique_ptr<Context> m_context;
};

enum
{
	ID_Quit = 1,
	ID_ResizeWideo,
	ID_About,
	ID_Debug_ffmpeg,
	ID_Debug_rtmp,
	ID_Debug_curl,
	IDB_SecInput,
	IDB_Seek,
	IDB_Read
};