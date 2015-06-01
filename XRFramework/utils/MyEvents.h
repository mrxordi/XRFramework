#pragma once
#include "wx/event.h"

class CDX10FrameRenderer;

class wxVideoRendererEvent : public wxEvent
{
public:
	enum VR_ACTION {
		VR_ACTION_ATTACH,
		VR_ACTION_DETACH,
		VR_ACTION_OPENING,
		VR_ACTION_OPENED,
		VR_ACTION_ABORTED,
		VR_ACTION_CLOSING,
	};
public:
	wxVideoRendererEvent(CDX10FrameRenderer* VideoRenderer, VR_ACTION action);
	virtual ~wxVideoRendererEvent();

	wxEvent* Clone() const override { return new wxVideoRendererEvent(*this); }
 	VR_ACTION e_action;
	CDX10FrameRenderer* pVideoRenderer;
};

wxDECLARE_EVENT(wxEVT_VIDEO_RENDERER, wxVideoRendererEvent);