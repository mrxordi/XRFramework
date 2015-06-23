#include "stdafxf.h"
#include "DemuxStream.h"

AVDiscard DemuxStream::GetDiscard()
{
   return AVDISCARD_NONE;
}

void DemuxStream::SetDiscard(AVDiscard discard)
{
   return;
}

void DemuxStreamVideo::GetStreamInfo(std::string& strInfo)
{
   if (!m_stream) return;
   char temp[128];
   avcodec_string(temp, 128, m_stream->codec, 0);
   strInfo = temp;
}

void DemuxStreamAudio::GetStreamType(std::string& strInfo)
{
   char sInfo[64];

   if (codec == AV_CODEC_ID_AC3) strcpy(sInfo, "AC3 ");
   else if (codec == AV_CODEC_ID_DTS)
   {
#ifdef FF_PROFILE_DTS_HD_MA
      if (profile == FF_PROFILE_DTS_HD_MA)
         strcpy(sInfo, "DTS-HD MA ");
      else if (profile == FF_PROFILE_DTS_HD_HRA)
         strcpy(sInfo, "DTS-HD HRA ");
      else
#endif
         strcpy(sInfo, "DTS ");
   }
   else if (codec == AV_CODEC_ID_MP2) strcpy(sInfo, "MP2 ");
   else if (codec == AV_CODEC_ID_TRUEHD) strcpy(sInfo, "Dolby TrueHD ");
   else strcpy(sInfo, "");

   if (iChannels == 1) strcat(sInfo, "Mono");
   else if (iChannels == 2) strcat(sInfo, "Stereo");
   else if (iChannels == 6) strcat(sInfo, "5.1");
   else if (iChannels == 8) strcat(sInfo, "7.1");
   else if (iChannels != 0)
   {
      char temp[32];
      sprintf(temp, " %d%s", iChannels, "-chs");
      strcat(sInfo, temp);
   }
   strInfo = sInfo;
}

void DemuxStreamAudio::GetStreamInfo(std::string& strInfo)
{
   if (!m_stream) return;
   char temp[128];
   avcodec_string(temp, 128, m_stream->codec, 0);
   strInfo = temp;
}

void DemuxStreamAudio::GetStreamName(std::string& strInfo)
{
   if (!m_stream) return;
   if (!m_description.empty())
      strInfo = m_description;
   else
      DemuxStream::GetStreamName(strInfo);
}
