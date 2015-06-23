#pragma once
#include "dvdplayer/Demuxers/DemuxStream.h"
#include "XRThreads/CriticalSection.h"
#include "XRThreads/SingleLock.h"

typedef struct 
{
   StreamType   type;
   int          type_index;
   std::string  name;
   std::string  filename;
   int          source;
   int          id;
   std::string  codec;
   int          channels;
} StreamEntry;

typedef std::vector<StreamEntry> EntriesVector;

class StreamsList
{
   XR::CCriticalSection m_section;
   StreamEntry  m_invalid;
public:
   StreamsList() {
      m_invalid.id = -1;
      m_invalid.source = STREAM_SOURCE_NONE;
      m_invalid.type = STREAM_NONE;
   };
   virtual ~StreamsList() = default;

   EntriesVector m_Streams;

   int              IndexOf(StreamType type, int source, int id) const;
   int              Count(StreamType type) const { return IndexOf(type, STREAM_SOURCE_NONE, -1) + 1; }
   int              CountSource(StreamType type, StreamSource source) const;
   StreamEntry&     Get(StreamType type, int index);

   EntriesVector Get(StreamType type);
   template<typename Compare> EntriesVector Get(StreamType type, Compare compare)
   {
      EntriesVector streams = Get(type);
      std::stable_sort(streams.begin(), streams.end(), compare);
      return streams;
   }

   void             Clear(StreamType type, StreamSource source);
   int              Source(StreamSource source, std::string filename);

   void             Update(StreamEntry& s);
   void             Update(IInputStream* input, Demuxer* demuxer);
};

