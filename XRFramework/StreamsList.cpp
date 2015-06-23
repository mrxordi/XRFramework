#include "stdafxf.h"
#include "StreamsList.h"
#include "XRCommon/log/Log.h"
#include "dvdplayer/Demuxers/Demux.h"

int StreamsList::IndexOf(StreamType type, int source, int id) const
{
   XR::CSingleLock lock(m_section);
   int count = -1;
   for (size_t i = 0; i < m_Streams.size(); i++)
   {
      if (type && m_Streams[i].type != type)
         continue;
      count++;
      if (source && m_Streams[i].source != source)
         continue;
      if (id < 0)
         continue;
      if (m_Streams[i].id == id)
         return count;
   }
   if (id < 0)
      return count;
   else
      return -1;
}

int StreamsList::CountSource(StreamType type, StreamSource source) const
{
   XR::CSingleLock lock(m_section);
   int count = 0;
   for (size_t i = 0; i < m_Streams.size(); i++)
   {
      if (type && m_Streams[i].type != type)
         continue;
      if (source && m_Streams[i].source != source)
         continue;
      count++;
      continue;
   }
   return count;
}

StreamEntry& StreamsList::Get(StreamType type, int index)
{
   XR::CSingleLock lock(m_section);
   int count = -1;
   for (size_t i = 0; i < m_Streams.size(); i++)
   {
      if (m_Streams[i].type != type)
         continue;
      count++;
      if (count == index)
         return m_Streams[i];
   }
   LOGERR("Failed to get stream!");
   return m_invalid;
}

void StreamsList::Clear(StreamType type, StreamSource source)
{
   XR::CSingleLock lock(m_section);
   for (int i = m_Streams.size() - 1; i >= 0; i--)
   {
      if (type && m_Streams[i].type != type)
         continue;

      if (source && m_Streams[i].source != source)
         continue;

      m_Streams.erase(m_Streams.begin() + i);
   }
}

int StreamsList::Source(StreamSource source, std::string filename)
{
   XR::CSingleLock lock(m_section);
   int index = source - 1;
   for (size_t i = 0; i < m_Streams.size(); i++)
   {
      StreamEntry &s = m_Streams[i];
      if (STREAM_SOURCE_MASK(s.source) != source)
         continue;
      // if it already exists, return same
      if (s.filename == filename)
         return s.source;
      if (index < s.source)
         index = s.source;
   }
   // return next index
   return index + 1;
}

void StreamsList::Update(IInputStream* input, Demuxer* demuxer)
{
   if (demuxer)
   {
      std::string filename = demuxer->GetFilename();
      int count = demuxer->GetNrOfStreams();
      int source;
      if (input) /* hack to know this is sub decoder */
         source = Source(STREAM_SOURCE_DEMUX, filename);

      for (int i = 0; i < count; i++)
      {
         DemuxStream* stream = demuxer->GetStream(i);
         /* skip streams with no type */
         if (stream->type == STREAM_NONE)
            continue;
         /* make sure stream is marked with right source */
         stream->source = source;

         StreamEntry s;
         s.source = source;
         s.type = stream->type;
         s.id = stream->iId;
         s.filename = demuxer->GetFilename();
         stream->GetStreamName(s.name);
         std::string codec;
         demuxer->GetStreamCodecName(stream->iId, codec);
         s.codec = codec;
         s.channels = 0; // Default to 0. Overwrite if STREAM_AUDIO below.
         if (stream->type == STREAM_AUDIO)
         {
            std::string type;
            ((DemuxStreamAudio*)stream)->GetStreamType(type);
            if (type.length() > 0)
            {
               if (s.name.length() > 0)
                  s.name += " - ";
               s.name += type;
            }
            s.channels = ((DemuxStreamAudio*)stream)->iChannels;
         }
         Update(s);
      }
   }
}

void StreamsList::Update(StreamEntry& s)
{
   XR::CSingleLock lock(m_section);
   int index = IndexOf(s.type, s.source, s.id);
   if (index >= 0)
   {
      StreamEntry& o = Get(s.type, index);
      s.type_index = o.type_index;
      o = s;
   }
   else
   {
      s.type_index = Count(s.type);
      m_Streams.push_back(s);
   }
}

