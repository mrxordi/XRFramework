#pragma once
#include "wx/wx.h" 
#include "wx/frame.h"
#include "wx/spinctrl.h"
#include <memory>
#include <stdint.h>
#include "XRThreads/Event.h"
#include "wxXRStatusBar.h"

struct Context;
class CPlayer;
class wxXRStatusBar;

class MainFrame : public wxFrame
{
public:

	MainFrame(const wxString& title, const wxRect& rect);
   virtual ~MainFrame();

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnResizeWideo(wxCommandEvent& event);
	void DebugToggle(wxCommandEvent& event);

	void OnCloseWindow(wxCloseEvent& event);

   wxXRStatusBar* GetBusyBar();
	
	DECLARE_EVENT_TABLE()

   virtual wxStatusBar* OnCreateStatusBar(int number = 1, long style = wxSTB_DEFAULT_STYLE, wxWindowID id = 0, const wxString& name = wxStatusLineNameStr) override;

	wxSizer* m_sizer;
	wxSpinCtrl* m_msInput;
   CPlayer* m_player;

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
   IDB_LoadTestFrame,
   IDB_Seek,
   IDB_Read,
};

enum {
   IDS_MainPart = 0,
   IDS_Waiting,
   IDS_Max
};
