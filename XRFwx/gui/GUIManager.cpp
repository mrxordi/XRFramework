#include "stdafxf.h"
#include "GUIManager.h"
#include "wxDX10Display.h"
#include "wx/msw/private/keyboard.h"
#include "XRCommon/utils/SpecialProtocol.h"
#include "XRFramework/render/RenderSystemDX.h"

#define EVT_MOUSE_(func) \
    EVT_LEFT_DOWN(func) \
    EVT_LEFT_UP(func) \
    EVT_LEFT_DCLICK(func) \
    EVT_MIDDLE_DOWN(func) \
    EVT_MIDDLE_UP(func) \
    EVT_MIDDLE_DCLICK(func) \
    EVT_RIGHT_DOWN(func) \
    EVT_RIGHT_UP(func) \
    EVT_RIGHT_DCLICK(func) \
    EVT_MOUSE_AUX1_DOWN(func) \
    EVT_MOUSE_AUX1_UP(func) \
    EVT_MOUSE_AUX1_DCLICK(func) \
    EVT_MOUSE_AUX2_DOWN(func) \
    EVT_MOUSE_AUX2_UP(func) \
    EVT_MOUSE_AUX2_DCLICK(func)
uint32_t virtualkey2scancode(WPARAM wParam, LPARAM lParam)
{
    if (HIWORD(lParam) & 0x0F00)
    {
        UINT scancode = MapVirtualKey(wParam, 0);
        return scancode | 0x80;
    }
    else
    {
        return HIWORD(lParam) & 0x00FF;
    }
}

BEGIN_EVENT_TABLE(CGUIManager, wxEvtHandler)
EVT_MOTION(CGUIManager::MouseMotion)
EVT_MOUSE_(CGUIManager::MouseKey)
EVT_MOUSEWHEEL(CGUIManager::MouseWheel)
EVT_KEY_UP(CGUIManager::HandleKeyUp)
EVT_KEY_DOWN(CGUIManager::HandleKeyDown)
EVT_CHAR_HOOK(CGUIManager::HandleInChar)
END_EVENT_TABLE()

IMPLEMENT_CLASS(CGUIManager, wxEvtHandler)

void CGUIManager::MouseMotion(wxMouseEvent& evt)
{
    LOGINFO("Mouse motion %ix%i", evt.GetX(), evt.GetY());
    CEGUI::System::getSingletonPtr()->getDefaultGUIContext().injectMousePosition(evt.GetX(), evt.GetY());
}

CGUIManager::CGUIManager(wxDX10Display* parent) : m_parent(parent)
{
    guilogger = new GUILogger();
    m_ceguiRenderer = &CEGUI::Direct3D10Renderer::create(m_parent->GetDXRenderer()->GetDevice());
    CEGUI::System::create(*m_ceguiRenderer);
    m_parent->PushEventHandler(this);

    CEGUI::SchemeManager::getSingleton().createFromFile(CSpecialProtocol::TranslatePath("special://app/data/schemes/TaharezLook.scheme"));
    CEGUI::FontManager::getSingleton().createFromFile("c:/Windows/Fonts/arial.ttf");
    CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultFont("arial");
    CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");
    CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultTooltipType("TaharezLook/Tooltip");

    m_bRenderGui = true;
}

CGUIManager::~CGUIManager()
{
    m_parent->PopEventHandler(false);
    CEGUI::System::destroy();
    CEGUI::Direct3D10Renderer::destroy(*static_cast<CEGUI::Direct3D10Renderer*>(m_ceguiRenderer));
    SAFE_DELETE(guilogger);
}

void CGUIManager::MouseKey(wxMouseEvent& evt)
{
    if (evt.ButtonDClick()) {
        CEGUI::System::getSingletonPtr()->getDefaultGUIContext().injectMouseButtonDoubleClick((CEGUI::MouseButton)evt.GetButton());
        return;
    }

    if (evt.ButtonDown()) {
        if (!m_parent->HasFocus())
            m_parent->SetFocus();
        CEGUI::System::getSingletonPtr()->getDefaultGUIContext().injectMouseButtonDown((CEGUI::MouseButton)evt.GetButton());
        return;
    }

    if (evt.ButtonUp()) {
        CEGUI::System::getSingletonPtr()->getDefaultGUIContext().injectMouseButtonUp((CEGUI::MouseButton)evt.GetButton());
        return;
    }
}

void CGUIManager::MouseWheel(wxMouseEvent& evt)
{
    int delta = evt.GetWheelRotation();
    float fdelta = delta * 0.05f;
    CEGUI::System::getSingletonPtr()->getDefaultGUIContext().injectMouseWheelChange(fdelta);
}

void CGUIManager::HandleKeyUp(wxKeyEvent& evt)
{
//     bool special = false;
//     uint32_t vsc = wxMSWKeyboard::WXToVK(evt.GetKeyCode(), &special);
//     vsc = MapVirtualKey(vsc, MAPVK_VK_TO_VSC);
//     vsc ^= 0xFF;
    uint32_t vsc = virtualkey2scancode(evt.GetRawKeyCode(), evt.GetRawKeyFlags());
    CEGUI::System::getSingletonPtr()->getDefaultGUIContext().injectKeyUp((CEGUI::Key::Scan)vsc);
}

void CGUIManager::HandleKeyDown(wxKeyEvent& evt)
{
    uint32_t vsc = virtualkey2scancode(evt.GetRawKeyCode(), evt.GetRawKeyFlags());
    CEGUI::System::getSingletonPtr()->getDefaultGUIContext().injectKeyDown((CEGUI::Key::Scan)vsc);
}

void CGUIManager::HandleInChar(wxKeyEvent& evt)
{
    CEGUI::System::getSingletonPtr()->getDefaultGUIContext().injectChar(static_cast<CEGUI::utf32>(evt.GetRawKeyCode()));
}

bool CGUIManager::RenderGui(uint32_t deltatime)
{
    if (!m_bRenderGui)
        return false;

    CEGUI::System& system = CEGUI::System::getSingleton();
    system.injectTimePulse(deltatime);

    system.renderAllGUIContexts();
}
