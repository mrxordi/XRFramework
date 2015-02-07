#include <stdafxf.h>
#include "wxDX10renderer.h"
#include "main.h"
#include <functional>
#include "../XRFramework/render/RenderSystemDX.h"
#include "../XRCommon/log/Log.h"

BEGIN_EVENT_TABLE(wxDX10renderer, wxWindow)
EVT_SIZE(wxDX10renderer::OnSizeEvent)
EVT_MOVE_START(wxDX10renderer::HandleEnterSizeMove)
EVT_MOVE_END(wxDX10renderer::HandleExitSizeMove)
END_EVENT_TABLE()

IMPLEMENT_CLASS(wxDX10renderer, wxWindow)


wxDX10renderer::wxDX10renderer(wxWindow *parent, wxWindowID id /*= wxID_ANY*/, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*= wxDefaultSize*/, long style /*= 0*/, const wxString& name /*= L"DX10Canvas"*/) 
	: m_bIsSizing(false)
{
	LOGDEBUG("DX10 render control created.");

	if (GetAppDX().get()) {
		GetAppDX()->DestroyRenderSystem();
	}
	GetAppDX().reset(new cRenderSystemDX);

	if (CreateWindow(parent, id, pos, size, style, name)) {
		wxGetApp().Bind(wxEVT_IDLE, &wxDX10renderer::OnIdle, this);
	}
}

bool wxDX10renderer::CreateWindow(wxWindow *parent, wxWindowID id /*= wxID_ANY*/, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*= wxDefaultSize*/, long style /*= 0*/, const wxString& name /*= L"DX10Canvas"*/)
{
	wxCHECK_MSG(parent, false, wxT("can't create wxWindow without parent"));

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

	if (!GetAppDX().get() || !GetAppDX()->InitRenderSystem(nullptr, GetHWND())) {
		LOGFATAL("Failed to initialize render system.")
		parent->RemoveChild(this);

		::DestroyWindow(GetHWND());
		return false;
	}

	Bind(wxEVT_PAINT, std::bind(&wxDX10renderer::Render, this));
	parent->Bind(wxEVT_MOVE_START, &wxDX10renderer::HandleEnterSizeMove, this);
	parent->Bind(wxEVT_MOVE_END, &wxDX10renderer::HandleExitSizeMove, this);
	//Bind(wxEVT_SIZE, &wxDX10renderer::OnSizeEvent, this);
	//Bind(wxEVT_MOVE_START, &wxDX10renderer::HandleEnterSizeMove, this);
	//Bind(wxEVT_MOVE_END, &wxDX10renderer::HandleExitSizeMove, this);
		
	return true;
}

wxDX10renderer::~wxDX10renderer()
{
	if (GetAppDX().get() && GetAppDX()->GetDevice())
		GetAppDX()->DestroyRenderSystem();

	LOGDEBUG("DX10 render control destroing.");
}

void wxDX10renderer::Render()
{
	if(GetAppDX()->BeginRender())
		GetAppDX()->EndRender();
}

void wxDX10renderer::OnSizeEvent(wxSizeEvent &event)
{
	if (m_bIsSizing)
		return;
	m_videoSize = event.GetSize();
	GetAppDX()->OnResize();
}

void wxDX10renderer::HandleEnterSizeMove(wxMoveEvent &event)
{
	m_bIsSizing = true;
	m_videoSize = GetSize();
}

void wxDX10renderer::HandleExitSizeMove(wxMoveEvent &event)
{
	m_bIsSizing = false;
	if (m_videoSize != GetSize() ) {
		GetAppDX()->OnResize();
		m_videoSize = GetSize();
	}
}

void wxDX10renderer::OnIdle(wxIdleEvent& event)
{
	Render();
	event.RequestMore();
}
