#pragma once

class IInputStream;
class Demuxer;

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

#define STREAM_SOURCE_MASK(a) ((a) & 0xf00)


enum StreamType
{
   STREAM_NONE = 0,// if unknown
   STREAM_AUDIO,   // audio stream
   STREAM_VIDEO,   // video stream
};

enum StreamSource {
   STREAM_SOURCE_NONE = 0x000,
   STREAM_SOURCE_DEMUX = 0x100,
   STREAM_SOURCE_NAV = 0x200,
   STREAM_SOURCE_DEMUX_SUB = 0x300,
   STREAM_SOURCE_TEXT = 0x400,
   STREAM_SOURCE_VIDEOMUX = 0x500
};

class DemuxStream
{
public:
   DemuxStream() {
      iId = 0;
      iPhysicalId = 0;
      codec = (AVCodecID)0; // AV_CODEC_ID_NONE
      codec_fourcc = 0;
      profile = FF_PROFILE_UNKNOWN;
      level = FF_LEVEL_UNKNOWN;
      type = STREAM_NONE;
      source = STREAM_SOURCE_NONE;
      iDuration = 0;
      pPrivate = NULL;
      ExtraData = NULL;
      ExtraSize = 0;
      memset(language, 0, sizeof(language));
      disabled = false;
      changes = 0;
      orig_type = 0;
   };
   ~DemuxStream()
   {
      delete[] ExtraData;
   };
   virtual void GetStreamInfo(std::string& strInfo)
   {
      strInfo = "";
   }
   virtual void GetStreamName(std::string& strInfo) 
   {
      strInfo = "";
   };

   virtual void      SetDiscard(AVDiscard discard);
   virtual AVDiscard GetDiscard();

   int iId;         // most of the time starting from 0
   int iPhysicalId; // id
   AVCodecID codec;
   unsigned int codec_fourcc; // if available
   int profile; // encoder profile of the stream reported by the decoder. used to qualify hw decoders.
   int level;   // encoder level of the stream reported by the decoder. used to qualify hw decoders.
   StreamType type;
   int source;

   int iDuration; // in mseconds
   void* pPrivate; // private pointer for the demuxer
   uint8_t*     ExtraData; // extra data for codec to use
   unsigned int ExtraSize; // size of extra data

   char language[4]; // ISO 639 3-letter language code (empty string if undefined)
   bool disabled; // set when stream is disabled. (when no decoder exists)

   int  changes; // increment on change which player may need to know about

   int orig_type; // type of original source
};

class DemuxStreamVideo : public DemuxStream
{
public:
   DemuxStreamVideo(Demuxer *parent, AVStream* stream) : DemuxStream(), m_parent(parent), m_stream(stream)
   {
      iFpsScale = 0;
      iFpsRate = 0;
      irFpsScale = 0;
      irFpsRate = 0;
      iHeight = 0;
      iWidth = 0;
      fAspect = 0.0;
      bVFR = false;
      bPTSInvalid = false;
      bForcedAspect = false;
      type = STREAM_VIDEO;
      iOrientation = 0;
      iBitsPerPixel = 0;
   }

   virtual ~DemuxStreamVideo() {}
   virtual void GetStreamInfo(std::string& strInfo);

   int iFpsScale; // scale of 1000 and a rate of 29970 will result in 29.97 fps
   int iFpsRate;
   int irFpsScale;
   int irFpsRate;
   int iHeight; // height of the stream reported by the demuxer
   int iWidth; // width of the stream reported by the demuxer
   float fAspect; // display aspect of stream
   bool bVFR;  // variable framerate
   bool bPTSInvalid; // pts cannot be trusted (avi's).
   bool bForcedAspect; // aspect is forced from container
   int iOrientation; // orientation of the video in degress counter clockwise
   int iBitsPerPixel;
   std::string stereo_mode; // expected stereo mode
private:
    Demuxer* m_parent;
   AVStream* m_stream;
};

class DemuxStreamAudio : public DemuxStream
{
public:
   DemuxStreamAudio(Demuxer *parent, AVStream* stream) : DemuxStream(), m_parent(parent), m_stream(stream)
   {
      iChannels = 0;
      iSampleRate = 0;
      iBlockAlign = 0;
      iBitRate = 0;
      iBitsPerSample = 0;
      type = STREAM_AUDIO;
   }

   virtual ~DemuxStreamAudio() {}

   void GetStreamType(std::string& strInfo);
   virtual void GetStreamInfo(std::string& strInfo);
   virtual void GetStreamName(std::string& strInfo);

   int iChannels;
   int iSampleRate;
   int iBlockAlign;
   int iBitRate;
   int iBitsPerSample;
   std::string m_description;

private:
   Demuxer* m_parent;
   AVStream* m_stream;
};