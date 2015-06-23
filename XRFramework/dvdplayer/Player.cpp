#include "stdafxf.h"
#include "Player.h"
#include "XRFwx/Context.h"
#include "XRFwx/MainFrame.h"
#include "XRFwx/wxXRStatusBar.h"


CPlayer::CPlayer(Context* ctx, wxEventHandler* parent) : CThread("CPlayerThread"), m_parent(parent), m_ready(new CEvent(true)), m_context(ctx), m_pInputStream(nullptr)
{
   m_bAbortRequested = false;
   m_pDemuxer = nullptr;
}


CPlayer::~CPlayer()
{
   Close();
}

bool CPlayer::Open(const CWeebTv::channel* channel)
{
   if (IsRunning())
      Close();
   if (!channel)
      return false;
   m_bAbortRequested = false;

   m_ready->Reset();
   m_channel = *channel;

   Create();
   m_context->frame->GetBusyBar()->WaitOnEvent(m_ready, "Opening wideo .", nullptr, 100, true);

   // Playback might have been stopped due to some error
   if (m_bStop || m_bAbortRequested)
      return false;

   return true;
}

void CPlayer::Process()
{
   if (!OpenInputStream())
   {
      m_bAbortRequested = true;
      return;
   }
   if (!OpenDemuxStream())
   {
      m_bAbortRequested = true;
      return;
   }

   m_ready->Set();
   while (!m_bStop) {

   }
}

bool CPlayer::OpenInputStream()
{
   if (m_pInputStream)
      m_pInputStream.reset();
   LOGNOTICE("Creating BufferedInputStream %s.", m_channel.title.c_str());

   CWeebTv* handler = m_channel.handler;

   std::string link = handler->GetStreamLink(m_channel.id_name, false); 
   //For now we use only buffered version of RTMP Stream
   m_pInputStream.reset (new CBufferedStream);

   if (!m_pInputStream->Open(link.c_str(), "flv"))
      return false;




   return true;
}

bool CPlayer::OpenDemuxStream()
{
   if (m_pDemuxer)
      SAFE_DELETE(m_pDemuxer);
   LOGNOTICE("Creating Demuxer...");

   std::unique_ptr<Demuxer> demuxer(new Demuxer());
   if (demuxer->Open(m_pInputStream))
      m_pDemuxer = demuxer.release();

   if (!m_pDemuxer)
   {
      LOGERR("Error creating demuxer");
      return false;
   }

   m_SelectionList.Clear(STREAM_NONE, STREAM_SOURCE_DEMUX);
   m_SelectionList.Update(m_pInputStream.get(), m_pDemuxer);

   return true;
}


void CPlayer::Close()
{
   StopThread();

   if (m_pInputStream.get())
      m_pInputStream->Close();
   m_pInputStream.reset();

   m_ready->Reset();
}
