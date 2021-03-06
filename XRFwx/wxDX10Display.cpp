#include <stdafxf.h>
#include "wxDX10Display.h"
#include "Context.h"
#include "main.h"
#include <functional>
#include "../XRFramework/render/RenderSystemDX.h"
#include "../XRFramework/core/VideoRenderers/DX10FrameRenderer.h"
#include "../XRCommon/log/Log.h"
#include "XRThreads/SystemClock.h"

BEGIN_EVENT_TABLE(wxDX10Display, wxWindow)
EVT_SIZE(wxDX10Display::OnSizeEvent)
EVT_MOVE_START(wxDX10Display::HandleEnterSizeMove)
EVT_MOVE_END(wxDX10Display::HandleExitSizeMove)
END_EVENT_TABLE()

IMPLEMENT_CLASS(wxDX10Display, wxWindow)


wxDX10Display::wxDX10Display(wxWindow *parent, Context* ctx, wxWindowID id /*= wxID_ANY*/, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*= wxDefaultSize*/, long style /*= 0*/, const wxString& name /*= L"DX10Canvas"*/)
: m_bIsSizing(false), m_context(ctx), m_pDXSystemRenderer(std::make_unique<CDX10SystemRenderer>(ctx))
{
	LOGDEBUG("DX10 render control created.");

	if (m_pDXSystemRenderer->GetDevice())
		m_pDXSystemRenderer->DestroyRenderSystem();

	if (CreateWindow(parent, id, pos, size, style, name))
		wxGetApp().Bind(wxEVT_IDLE, &wxDX10Display::OnIdle, this);
}

bool wxDX10Display::CreateWindow(wxWindow *parent, wxWindowID id /*= wxID_ANY*/, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*= wxDefaultSize*/, long style /*= 0*/, const wxString& name /*= L"DX10Canvas"*/)
{
	wxCHECK_MSG(parent, false, wxT("can't create wxWindow without parent"));
    style = style | wxWANTS_CHARS;
	if (!CreateBase(parent, id, pos, size, style, wxDefaultValidator, name))
		return false;

	parent->AddChild(this);

	/*
	A general rule with OpenGL and Win32 is that any window that will have a
	HGLRC built for it must have two flags:  WS_CLIPCHILDREN & WS_CLIPSIBLINGS.
	You can find references about this within the knowledge base and most OpenGL
	books that contain the wgl function descriptions.
	*/
	WXDWORD exStyle = 0;
	DWORD msflags = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	msflags |= MSWGetStyle(style, &exStyle);

	if (!MSWCreate(wxApp::GetRegisteredClassName(wxT("wDX10renderer"), -1, CS_OWNDC),
		NULL, pos, size, msflags, exStyle))
		return false;

	if (!m_pDXSystemRenderer.get() || !m_pDXSystemRenderer->InitRenderSystem(GetHWND()))
	{
		LOGFATAL("Failed to initialize render system.")
			parent->RemoveChild(this);

		::DestroyWindow(GetHWND());
		return false;
	}

	Bind(wxEVT_PAINT, std::bind(&wxDX10Display::Render, this));
	parent->Bind(wxEVT_MOVE_START, &wxDX10Display::HandleEnterSizeMove, this);
	parent->Bind(wxEVT_MOVE_END, &wxDX10Display::HandleExitSizeMove, this);
	Bind(wxEVT_VIDEO_RENDERER, &wxDX10Display::OnVideoRendererEvent, this);

   m_context->videoDisplay = this;

   m_pVideoRenderer = std::make_unique<CDX10FrameRenderer>(m_context);
   //To Tylko w czasie tworzenia
   m_pVideoRenderer->Configure(1920, 1080, 24.0, CONF_FLAGS_YUVCOEF_BT601, RENDER_FMT_YUV420P, this);

   m_context->frameRenderer = m_pVideoRenderer.get();
   m_pGUIManager = std::make_unique<CGUIManager>(this);
   
	return true;
}

wxDX10Display::~wxDX10Display()
{
    m_pGUIManager.reset();
	if (m_pDXSystemRenderer.get() && m_pDXSystemRenderer->GetDevice())
	{
		if (m_pVideoRenderer)
			m_pVideoRenderer.reset();
		m_pDXSystemRenderer->DestroyRenderSystem();
	}

	LOGDEBUG("DX10 render control destroing.");
}

void wxDX10Display::Render()
{
    uint32_t deltatime = XR::SystemClockMillis();
	if (m_pDXSystemRenderer->BeginRender())
	{
		if (m_pVideoRenderer)
			m_pVideoRenderer->Render();
		else
		{

		}
        if (m_pGUIManager)
        {
            if (!m_pGUIManager->RenderGui(deltatime))
            {
                //empty for now
            }
        }
		m_pDXSystemRenderer->EndRender();
	}
}

void wxDX10Display::OnSizeEvent(wxSizeEvent &event)
{
#ifdef _DEBUG
	if (m_bIsSizing)  //	That is uncommented only when debugging and coding ;)
		return;			//	It isn't needed (slows app)
#endif
	m_videoSize = event.GetSize();
	m_pDXSystemRenderer->OnResize();
}

void wxDX10Display::HandleEnterSizeMove(wxMoveEvent &event)
{
	m_bIsSizing = true;
	m_videoSize = GetSize();
}

void wxDX10Display::HandleExitSizeMove(wxMoveEvent &event)
{
	m_bIsSizing = false;
	if (m_videoSize != GetSize()) 
	{
		m_pDXSystemRenderer->OnResize();
		m_videoSize = GetSize();
	}
}

void wxDX10Display::OnIdle(wxIdleEvent& event)
{
	Render();
	event.RequestMore();
}

void wxDX10Display::OnVideoRendererEvent(wxVideoRendererEvent &event)
{
   if (event.pVideoRenderer)
      XR::CSingleLock lock(event.pVideoRenderer->m_eventlock);
   else
      return;

	switch (event.e_action) 
	{
	case wxVideoRendererEvent::VR_ACTION_ATTACH:
   {
      break;
   }
	case wxVideoRendererEvent::VR_ACTION_DETACH:
		m_pVideoRenderer = nullptr;
		break;
	};
}