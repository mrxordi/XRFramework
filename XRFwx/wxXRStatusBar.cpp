#include "stdafxf.h"
#include "wxXRStatusBar.h"
#include "wx/evtloop.h"
#include "wx/wx.h"
#include "main.h"


BEGIN_EVENT_TABLE(wxXRStatusBar, wxStatusBar)

END_EVENT_TABLE()

IMPLEMENT_CLASS(wxXRStatusBar, wxStatusBar)

wxXRStatusBar::wxXRStatusBar(wxWindow *parent,
   wxWindowID id, long style, const wxString& name) 
   : wxStatusBar(parent, id, style, name), CThread("StatusbarWait"), m_done(new CEvent()),
   m_animateChars({ { '/', '-', '\\', '|' } }), m_waitEvent(nullptr), m_orginalMessage(""), m_additionalMessage("")
{
   m_caller = nullptr;
   m_waitEvent = nullptr;
   int widths[IDS_Max];
   if (IDS_Max > 1) {
      for (int i = 0; i < IDS_Max -1; i++)
      {
         widths[i] = -1;
      }
      widths[IDS_Waiting] = 150;
   }
   SetFieldsCount(IDS_Max);
   SetStatusWidths(IDS_Max, widths);

   SetStatusText("Witam w XRFramework!", IDS_MainPart);
   //Set this for case if none of events will occur.
   m_done->Set();
}


wxXRStatusBar::~wxXRStatusBar()
{
/*   XR::CSingleLock lock(m_critSection);*/

   /*m_done->Set();*/
   CThread::StopThread(true);
   m_done->Wait();
}

bool wxXRStatusBar::WaitOnEvent(std::shared_ptr<CEvent> event, std::string message, wxEvtHandler* caller, uint32_t displaytime /*= 100*/, bool holdCaller /*= false*/)
{
   XR::CSingleLock lock(m_critSection);
   if (m_waitEvent)
      return false;
   m_done->Reset();

   m_waitEvent = event;
   if (!event->WaitMSec(displaytime))
   {
      m_orginalMessage = message;
      if (caller != nullptr) {
         caller->Bind(wxEVT_PROGRESS, &wxXRStatusBar::OnProgress, this);
         m_caller = caller;
      }
      else {
         m_parent->Bind(wxEVT_PROGRESS, &wxXRStatusBar::OnProgress, this);
         m_caller = m_parent;
      }

      CThread::Create(false);
   } else
      return false;
   if (holdCaller) {
      lock.unlock();
      while (!m_done->WaitMSec(1)) {
         wxGetApp().Yield();
      }
   }
   
   return true;
}

void wxXRStatusBar::Process()
  {
     int frame = 0;
      // throw up the progress
     while (/*!m_done->WaitMSec(1) &&*/ AbortableWait((*m_waitEvent), 200) == WAIT_TIMEDOUT)
     {
        XR::CSingleLock lock(m_critSection);

        frame = (frame == 4) ? 0 : frame;
        wxString msg = m_orginalMessage;
        if (!m_additionalMessage.empty())
           msg += (" " + m_additionalMessage);

        SetStatusText(msg +" "+ m_animateChars[frame], IDS_Waiting);
        frame++;
     }
     m_done->Set();
}

void wxXRStatusBar::OnProgress(wxProgressEvent& event)
{
   if (event.pHandler != m_caller)
      return;
   if (!m_bStop && !event.sString.empty())
      m_additionalMessage = std::move(event.sString);
}

void wxXRStatusBar::OnExit()
{
   m_waitEvent = nullptr;

   if (m_caller != nullptr)
      m_caller->Unbind(wxEVT_PROGRESS, &wxXRStatusBar::OnProgress, this);
   else
      m_parent->Unbind(wxEVT_PROGRESS, &wxXRStatusBar::OnProgress, this);
   if (!m_bStop)
      SetStatusText(wxString("Idle."), IDS_Waiting);
   m_done->Set();
}
