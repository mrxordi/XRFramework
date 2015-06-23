#pragma once
#include <memory>
#include <map>
#include <vector>
#include "XRThreads/SystemClock.h"
#include "dvdplayer/Demuxers/DemuxStream.h"
#include "StreamsList.h"

extern "C" {
#include "libavformat/avformat.h"
}

class IInputStream;

class Demuxer {
public:
   Demuxer();
   ~Demuxer();

   bool Open(std::shared_ptr<IInputStream> pInput, bool streaminfo = true);
   void Dispose();


   bool Aborted();

   int          GetStreamLength();
   void         GetStreamCodecName(int iStreamId, std::string &strName);
   DemuxStream* GetStream(int iStreamId);
   int          GetNrOfStreams();
   std::string  GetFilename();

   AVFormatContext* m_pFormatContext;
   std::shared_ptr<IInputStream> m_pInput;

protected:

   DemuxStream* AddStream(int iId);
   void AddStream(int iId, DemuxStream* stream);

   void CreateStreams();
   void DisposeStreams();


   XR::CCriticalSection m_critSection;
   std::map<int, DemuxStream*> m_streams;
   std::vector<std::map<int, DemuxStream*>::iterator> m_stream_index;


   AVIOContext* m_ioContext;
   XR::EndTime  m_timeout;
   double   m_currentPts; // used for stream length estimation

   // Due to limitations of ffmpeg, we only can detect a program change
   // with a packet. This struct saves the packet for the next read and
   // signals STREAMCHANGE to player
   struct
   {
      AVPacket pkt;       // packet ffmpeg returned
      int      result;    // result from av_read_packet
   }m_pkt;
   bool m_streaminfo;
   bool m_checkvideo;
};