#pragma once
#include "XRThreads/Thread.h"
#include "XRThreads/Event.h"
#include "wx/event.h"
#include "XRFwx/WeebTv.h"
#include "XRFwx/BufferedStream.h"
#include "dvdplayer/Demuxers/Demux.h"
#include "StreamsList.h"

struct Context;
class wxEventHandler;

class CPlayer : public CThread
{
public:
   CPlayer(Context* ctx, wxEventHandler* parent);
   virtual ~CPlayer();

   bool Open(const CWeebTv::channel* channel);
   void Close();

   bool IsPlaying() const { return !m_bStop; };
   bool OpenInputStream();
   bool OpenDemuxStream();


   virtual void Process() override;
private:
   bool m_bAbortRequested;

   StreamsList m_SelectionList;

   Context* m_context;
   /// Event handler to send Player events 
   wxEventHandler* m_parent;

   Demuxer *m_pDemuxer; // demuxer for current playing file
   std::shared_ptr<IInputStream> m_pInputStream; //Our Buffered Stream 

   CWeebTv::channel m_channel;
   std::shared_ptr<CEvent> m_ready;

};

