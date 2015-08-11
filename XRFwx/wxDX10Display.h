#pragma once
#include "wx/wx.h"
#include "gui/GUIManager.h"
#include "../XRFramework/utils/MyEvents.h"
#include <memory>


class CDX10SystemRenderer;
class CDX10FrameRenderer;
struct Context;

class wxDX10Display final : public wxWindow
{
   friend class CDX10FrameRenderer;
   friend class CGUIManager;
public:
	wxEXPLICIT wxDX10Display(wxWindow *parent, Context* ctx,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = L"DX10Canvas");

	virtual ~wxDX10Display();

	void Render();
   CDX10SystemRenderer* GetDXRenderer() { if (m_pDXSystemRenderer) return m_pDXSystemRenderer.get(); else return nullptr; }
   CGUIManager* GetGUIManager() { if (m_pGUIManager) return m_pGUIManager.get(); else return nullptr; }

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
	/// @brief We need to handle Video Renderer events
	void OnVideoRendererEvent(wxVideoRendererEvent &event);

	wxSize m_videoSize;
	wxSize m_oldvideoSize;
	bool   m_bIsSizing;
	std::unique_ptr<CDX10SystemRenderer> m_pDXSystemRenderer;
    std::unique_ptr<CDX10FrameRenderer> m_pVideoRenderer;
    std::unique_ptr<CGUIManager> m_pGUIManager;
	Context*			m_context;

	DECLARE_EVENT_TABLE()
	DECLARE_CLASS(wxDX10Display)
};

