#include "stdafxf.h"
#include "MyEvents.h"
#include "core/VideoRenderers/DX10FrameRenderer.h"
#include "../XRCommon/log/Log.h"

wxVideoRendererEvent::wxVideoRendererEvent(CDX10FrameRenderer* VideoRenderer, VR_ACTION action) 
	: pVideoRenderer(VideoRenderer), e_action(action)
{
	SetEventType(wxEVT_VIDEO_RENDERER);
}

wxVideoRendererEvent::~wxVideoRendererEvent()
{
	LOGINFO("render event destruction");
}

wxDEFINE_EVENT(wxEVT_VIDEO_RENDERER, wxVideoRendererEvent);


wxPlayerEvent::wxPlayerEvent(CPlayer* player) 
   : pPlayer(player)
{
   SetEventType(wxEVT_PLAYER);
}

wxPlayerEvent::~wxPlayerEvent()
{
   LOGINFO("player event destruction");
}

wxDEFINE_EVENT(wxEVT_PLAYER, wxPlayerEvent); 
wxDEFINE_EVENT(wxEVT_PLAYER_SEEK, wxPlayerSeekEvent); 
wxDEFINE_EVENT(wxEVT_PROGRESS, wxProgressEvent);