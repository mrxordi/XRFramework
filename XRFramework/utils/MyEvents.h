#pragma once
#include "wx/event.h"

class CDX10FrameRenderer;
class CPlayer;
class wxVideoRendererEvent;
class wxPlayerEvent;
class wxPlayerSeekEvent;
class wxProgressEvent;

wxDECLARE_EVENT(wxEVT_VIDEO_RENDERER, wxVideoRendererEvent);
wxDECLARE_EVENT(wxEVT_PLAYER, wxPlayerEvent);
wxDECLARE_EVENT(wxEVT_PLAYER_SEEK, wxPlayerSeekEvent);
wxDECLARE_EVENT(wxEVT_PROGRESS, wxProgressEvent);

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
      VR_ACTION_FRAME_READY
	};
public:
	wxVideoRendererEvent(CDX10FrameRenderer* VideoRenderer, VR_ACTION action);
	virtual ~wxVideoRendererEvent();

	wxEvent* Clone() const override { return new wxVideoRendererEvent(*this); }
 	VR_ACTION e_action;
	CDX10FrameRenderer* pVideoRenderer;
};


class wxPlayerEvent : public wxEvent
{
public:
   wxPlayerEvent(CPlayer* player);
   virtual ~wxPlayerEvent();

   wxEvent* Clone() const override { return new wxPlayerEvent(*this); }

   CPlayer* pPlayer;
};


class wxPlayerSeekEvent 
   : wxPlayerEvent
{
public:
   wxPlayerSeekEvent(CPlayer* player, int64_t offset) :wxPlayerEvent(player), iOffset(offset) 
   { SetEventType(wxEVT_PLAYER_SEEK); };
   ~wxPlayerSeekEvent() {};

   wxEvent* Clone() const override { return new wxPlayerSeekEvent(*this); }

private:
   int64_t iOffset;
};


class wxProgressEvent : public wxEvent {
public:
   wxProgressEvent(std::string& msg, wxEvtHandler* handler = nullptr) : sString(std::move(msg)), pHandler(handler)
      { SetEventType(wxEVT_PROGRESS); };

   wxEvent* Clone() const override { return new wxProgressEvent(*this); }
   std::string sString;
   wxEvtHandler* pHandler;
};



typedef void (wxEvtHandler::*wxStatusBarProgressHandler)(wxProgressEvent&);
#define wxStatusBarProgressHandler(func) \
    wxEVENT_HANDLER_CAST(wxStatusBarProgressHandler, func)
#define EVT_BAR_PROGRESS(func)  wx__DECLARE_EVT0(wxEVT_PROGRESS, wxStatusBarProgressHandler(func))
