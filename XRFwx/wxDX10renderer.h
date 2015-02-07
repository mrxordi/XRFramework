#pragma once
#include "wx/wx.h"
#include <memory>

class cRenderSystemDX;

class wxDX10renderer : public wxWindow
{

public:
	wxEXPLICIT wxDX10renderer(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = L"DX10Canvas");

	virtual ~wxDX10renderer();

	void Render();

protected:

	// the real window creation function
	bool CreateWindow(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = L"DX10Canvas");
private:
	/// @brief We need to handle OnIdle
	void OnIdle(wxIdleEvent& event);
	/// @brief We need to handle WM_ENTERSIZEMOVE and WM_EXITSIZEMOVE
	void HandleEnterSizeMove(wxMoveEvent &event);
	void HandleExitSizeMove(wxMoveEvent &event);
	/// @brief Recalculate video positioning and scaling when the available area or zoom changes
	void OnSizeEvent(wxSizeEvent &event);

	wxSize m_videoSize;
	wxSize m_oldvideoSize;
	bool   m_bIsSizing;

	DECLARE_EVENT_TABLE()
	DECLARE_CLASS(wxDX10renderer)
};

