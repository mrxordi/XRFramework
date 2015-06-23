#include "stdafxf.h"
#include "Demux.h"
#include "XRCommon/utils/StringUtils.h"
#include "dvdplayer/InputSteams/IInputStream.h"

#define FFMPEG_FILE_BUFFER_SIZE   32768 // default reading size for ffmpeg

static int interrupt_cb(void* ctx)
{
   Demuxer* demuxer = static_cast<Demuxer*>(ctx);
   if (demuxer && demuxer->Aborted())
      return 1;
   return 0;
}
static int dvd_file_read(void *h, uint8_t* buf, int size)
{
   if (interrupt_cb(h))
      return AVERROR_EXIT;

   IInputStream* pInputStream = static_cast<Demuxer*>(h)->m_pInput.get();
   return pInputStream->Read(buf, size);
}
static int64_t dvd_file_seek(void *h, int64_t pos, int whence)
{
   if (interrupt_cb(h))
      return AVERROR_EXIT;

   IInputStream* pInputStream = static_cast<Demuxer*>(h)->m_pInput.get();
   if (whence == AVSEEK_SIZE)
      return pInputStream->GetLength();
   else
      return pInputStream->Seek(pos, whence & ~AVSEEK_FORCE);
}
static double SelectAspect(AVStream* st, bool* forced)
{
   *forced = false;
   /* if stream aspect is 1:1 or 0:0 use codec aspect */
   if ((st->sample_aspect_ratio.den == 1 || st->sample_aspect_ratio.den == 0)
      && (st->sample_aspect_ratio.num == 1 || st->sample_aspect_ratio.num == 0)
      && st->codec->sample_aspect_ratio.num != 0)
      return av_q2d(st->codec->sample_aspect_ratio);

   *forced = true;
   if (st->sample_aspect_ratio.num != 0)
      return av_q2d(st->sample_aspect_ratio);

   return 0.0;
}

Demuxer::Demuxer()
{
   m_pFormatContext = nullptr;
   m_pInput = nullptr;
   m_ioContext = nullptr;
   m_pkt.result = -1;
   memset(&m_pkt.pkt, 0, sizeof(AVPacket));

   m_streaminfo = true; /* set to true if we want to look for streams before playback */
   m_checkvideo = false;
}

Demuxer::~Demuxer()
{

}


bool Demuxer::Open(std::shared_ptr<IInputStream> pInput, bool streaminfo /*= true*/)
{
   AVInputFormat* iformat = NULL;
   std::string strFile;
   const AVIOInterruptCB int_cb = { interrupt_cb, this };
   m_streaminfo = streaminfo;
   m_pInput = pInput;

   if (!m_pInput.get())
      return false;
   strFile = m_pInput->GetFileName();

   m_pFormatContext = avformat_alloc_context();
   m_pFormatContext->interrupt_callback = int_cb;

   if (!m_pInput->IsStreamType(DVDSTREAM_TYPE_RTMPBUFFERED))
      return false;

   unsigned char* buffer = (unsigned char*)av_malloc(FFMPEG_FILE_BUFFER_SIZE);
   m_ioContext = avio_alloc_context(buffer, FFMPEG_FILE_BUFFER_SIZE, 0, this, dvd_file_read, NULL, dvd_file_seek);
   if (m_ioContext->max_packet_size)
      m_ioContext->max_packet_size *= FFMPEG_FILE_BUFFER_SIZE / m_ioContext->max_packet_size;

   if (m_pInput->Seek(0, SEEK_POSSIBLE) == 0)
      m_ioContext->seekable = 0;

   std::string content = m_pInput->GetContent();
   if (content.compare("video/flv") != 0)
      iformat = av_find_input_format("flv");

   if (!iformat)
   {
      LOGERR("Error probing input format, %s", CURL::GetRedacted(strFile).c_str());
      return false;
   }
   else
   {
      if (iformat->name){
         LOGDEBUG("Probing detected format [%s]", iformat->name);
      }
      else{
         LOGDEBUG("Probing detected unnamed format");
      }
   }
   m_pFormatContext->pb = m_ioContext;

   AVDictionary *options = NULL;
   if (iformat->name && (strcmp(iformat->name, "mp3") == 0 || strcmp(iformat->name, "mp2") == 0))
   {
      LOGDEBUG("Setting usetoc to 0 for accurate VBR MP3 seek.");
      av_dict_set(&options, "usetoc", "0", 0);
   }

   if (avformat_open_input(&m_pFormatContext, strFile.c_str(), iformat, &options) < 0)
   {
      LOGERR("Error, could not open file %s", CURL::GetRedacted(strFile).c_str());
      Dispose();
      av_dict_free(&options);
      return false;
   }
   av_dict_free(&options);

   if (!iformat || (strcmp(iformat->name, "mpegts") != 0))
   {
      m_streaminfo = true;
   }

   if (m_streaminfo) {
      LOGDEBUG("avformat_find_stream_info starting.");
      int iErr = avformat_find_stream_info(m_pFormatContext, NULL);
      
      if (iErr < 0)
      {
         LOGWARN("Could not find codec parameters for %s", CURL::GetRedacted(strFile).c_str());
         Dispose();
      }
      LOGDEBUG("av_find_stream_info finished");
   }
   // reset any timeout
   m_timeout.SetInfinite();

   // if format can be nonblocking, let's use that
   m_pFormatContext->flags |= AVFMT_FLAG_NONBLOCK;

   // print some extra information
   av_dump_format(m_pFormatContext, 0, strFile.c_str(), 0);

   CreateStreams();
   return true;
}

bool Demuxer::Aborted()
{
   return false;
}

std::string Demuxer::GetFilename()
{
   if (m_pInput)
      return m_pInput->GetFileName();
   else
      return "";
}

int Demuxer::GetNrOfStreams()
{
   return m_stream_index.size();
}

DemuxStream* Demuxer::GetStream(int iStreamId)
{
   if (iStreamId >= 0 && (size_t)iStreamId < m_stream_index.size())
      return m_stream_index[iStreamId]->second;
   else
      return NULL;
}

void Demuxer::GetStreamCodecName(int iStreamId, std::string &strName)
{
   DemuxStream *stream = GetStream(iStreamId);
   if (stream)
   {
      unsigned int in = stream->codec_fourcc;
      // FourCC codes are only valid on video streams, audio codecs in AVI/WAV
      // are 2 bytes and audio codecs in transport streams have subtle variation
      // e.g AC-3 instead of ac3
      if (stream->type == STREAM_VIDEO && in != 0)
      {
         char fourcc[5];
         memcpy(fourcc, &in, 4);

         fourcc[4] = 0;
         // fourccs have to be 4 characters
         if (strlen(fourcc) == 4)
         {
            strName = fourcc;
            StringUtils::ToLower(strName);
            return;
         }
      }

      AVCodec *codec = avcodec_find_decoder(stream->codec);
      if (codec)
         strName = codec->name;
   }
}

int Demuxer::GetStreamLength()
{
   if (!m_pFormatContext)
      return 0;

   if (m_pFormatContext->duration < 0)
      return 0;

   return (int)(m_pFormatContext->duration / (AV_TIME_BASE / 1000));
}

void Demuxer::Dispose()
{
   m_pkt.result = -1;
   av_free_packet(&m_pkt.pkt);

   if (m_pFormatContext)
   {
      for (unsigned int i = 0; i < m_pFormatContext->nb_streams; i++)
      {
         avcodec_close(m_pFormatContext->streams[i]->codec);
      }

      if (m_ioContext && m_pFormatContext->pb && m_pFormatContext->pb != m_ioContext)
      {
         LOGWARN("Demuxer::Dispose - demuxer changed our byte context behind our back, possible memleak");
         m_ioContext = m_pFormatContext->pb;
      }
      avformat_close_input(&m_pFormatContext);
   }

   if (m_ioContext)
   {
      av_free(m_ioContext->buffer);
      av_free(m_ioContext);
   }

   m_ioContext = NULL;
   m_pFormatContext = NULL;

   DisposeStreams();

   m_pInput = NULL;
}

void Demuxer::DisposeStreams()
{

}

void Demuxer::CreateStreams()
{
   DisposeStreams();

   for (unsigned int i = 0; i < m_pFormatContext->nb_streams; i++)
      AddStream(i);
}

DemuxStream* Demuxer::AddStream(int iId)
{
   AVStream* pStream = m_pFormatContext->streams[iId];

   if (pStream)
   {
      DemuxStream* stream = NULL;

      switch (pStream->codec->codec_type)
      {
      case AVMEDIA_TYPE_AUDIO:
      {
         DemuxStreamAudio* st = new DemuxStreamAudio(this, pStream);
         stream = st;
         st->iChannels = pStream->codec->channels;
         st->iSampleRate = pStream->codec->sample_rate;
         st->iBlockAlign = pStream->codec->block_align;
         st->iBitRate = pStream->codec->bit_rate;
         st->iBitsPerSample = pStream->codec->bits_per_raw_sample;
         if (st->iBitsPerSample == 0)
            st->iBitsPerSample = pStream->codec->bits_per_coded_sample;
         if (av_dict_get(pStream->metadata, "title", NULL, 0))
            st->m_description = av_dict_get(pStream->metadata, "title", NULL, 0)->value;

         break;
      }
      case  AVMEDIA_TYPE_VIDEO:
      {
         DemuxStreamVideo *st = new DemuxStreamVideo(this, pStream);
         stream = st;
         if (strcmp(m_pFormatContext->iformat->name, "flv") == 0)
            st->bVFR = true;
         else
            st->bVFR = false;

         AVRational r_frame_rate = pStream->r_frame_rate;
         if (r_frame_rate.den && r_frame_rate.num)
         {
            st->iFpsRate = r_frame_rate.num;
            st->iFpsScale = r_frame_rate.den;
         }
         else
         {
            st->iFpsRate = 0;
            st->iFpsScale = 0;
         }

         st->iWidth = pStream->codec->width;
         st->iHeight = pStream->codec->height;
         st->fAspect = SelectAspect(pStream, &st->bForcedAspect) * pStream->codec->width / pStream->codec->height;
         st->iOrientation = 0;
         st->iBitsPerPixel = pStream->codec->bits_per_coded_sample;

         AVDictionaryEntry *rtag = av_dict_get(pStream->metadata, "rotate", NULL, 0);
         if (rtag)
            st->iOrientation = atoi(rtag->value);

         break;
      }
      default:
      {
         stream = new DemuxStream();
         stream->type = STREAM_NONE;
         break;
      }
      }
      // set ffmpeg type
      stream->orig_type = pStream->codec->codec_type;

      // generic stuff
      if (pStream->duration != (int64_t)AV_NOPTS_VALUE)
         stream->iDuration = (int)((pStream->duration / AV_TIME_BASE) & 0xFFFFFFFF);

      stream->codec = pStream->codec->codec_id;
      stream->codec_fourcc = pStream->codec->codec_tag;
      stream->profile = pStream->codec->profile;
      stream->level = pStream->codec->level;

      stream->source = STREAM_SOURCE_DEMUX;
      stream->pPrivate = pStream;

      if (pStream->codec->extradata && pStream->codec->extradata_size > 0)
      {
         stream->ExtraSize = pStream->codec->extradata_size;
         stream->ExtraData = new uint8_t[pStream->codec->extradata_size];
         memcpy(stream->ExtraData, pStream->codec->extradata, pStream->codec->extradata_size);
      }
      stream->iPhysicalId = pStream->id;
      AddStream(iId, stream);
      return stream;

   }
   else
      return nullptr;
}

void Demuxer::AddStream(int iId, DemuxStream* stream)
{
   std::pair<std::map<int, DemuxStream*>::iterator, bool> res;

   res = m_streams.insert(std::make_pair(iId, stream));
   if (res.second)
   {
      /* was new stream */
      stream->iId = m_stream_index.size();
      m_stream_index.push_back(res.first);
   }
   else
   {
      /* replace old stream, keeping old index */
      stream->iId = res.first->second->iId;

      delete res.first->second;
      res.first->second = stream;
   }
      LOGDEBUG("Demuxer::AddStream(%d, ...) -> %d", iId, stream->iId);
}
