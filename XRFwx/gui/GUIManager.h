#pragma once
#include "wx/wx.h"
#include "GUILogger.h"
#include "CEGUI/CEGUI.h"
#include "CEGUI/RendererModules/Direct3D10/Renderer.h"
class wxDX10Display;


class CGUIManager : wxEvtHandler
{
public:
    CGUIManager(wxDX10Display* parent);
    ~CGUIManager();

    bool RenderGui(uint32_t deltatime);

    void MouseMotion(wxMouseEvent& evt);
    void MouseKey(wxMouseEvent& evt);
    void MouseWheel(wxMouseEvent& evt);

    void HandleKeyUp(wxKeyEvent& evt);
    void HandleKeyDown(wxKeyEvent& evt);
    void HandleInChar(wxKeyEvent& evt);

private:
    GUILogger* guilogger;
    CEGUI::Renderer* m_ceguiRenderer;
    wxDX10Display* m_parent;

    bool m_bRenderGui;

    DECLARE_EVENT_TABLE()

    DECLARE_CLASS(CGUIManager)
};

