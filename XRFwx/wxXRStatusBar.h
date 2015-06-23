#pragma once
#include <array>
#include "XRThreads\Thread.h"
#include "XRThreads\Event.h"
#include "wx\statusbr.h"
#include "wx\event.h"
#include "MainFrame.h"
#include "XRFramework/utils/MyEvents.h"



class wxXRStatusBar :
   public wxStatusBar, public CThread
{
public:
   wxXRStatusBar(wxWindow *parent,
      wxWindowID id = wxID_ANY,
      long style = wxSTB_DEFAULT_STYLE,
      const wxString& name = wxStatusBarNameStr);

   virtual ~wxXRStatusBar();
   virtual bool WaitOnEvent(std::shared_ptr<CEvent> event, std::string message, wxEvtHandler *caller = nullptr, uint32_t displaytime = 100, bool holdCaller = false);
   virtual void OnProgress(wxProgressEvent& event);

   DECLARE_EVENT_TABLE()

private:
   virtual void Process() override;

   std::string m_orginalMessage;
   std::string m_additionalMessage;

   uint32_t m_frame;
   std::shared_ptr<CEvent> m_waitEvent;
   std::shared_ptr<CEvent> m_done;
   const std::array<char, 4> m_animateChars;
   XR::CCriticalSection m_critSection;
   wxEvtHandler *m_caller;

   DECLARE_CLASS(wxXRStatusBar)

   virtual void OnExit() override;

};

