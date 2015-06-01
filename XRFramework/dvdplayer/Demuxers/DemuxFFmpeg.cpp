#include "stdafxf.h"
#include "DemuxFFmpeg.h"
#include "dvdplayer/InputSteams/IInputStream.h"

void CDemuxStreamVideoFFmpeg::GetStreamInfo(std::string& strInfo)
{
	if (!m_stream) return;
	char temp[128];
	avcodec_string(temp, 128, m_stream->codec, 0);
	strInfo = temp;
}

void CDemuxStreamAudioFFmpeg::GetStreamName(std::string& strInfo)
{
	if (!m_stream) return;
	if (!m_description.empty())
		strInfo = m_description;
	else
		CDemuxStream::GetStreamName(strInfo);
}

static int interrupt_cb(void* ctx)
{
	CDemuxFFmpeg* demuxer = static_cast<CDemuxFFmpeg*>(ctx);
	if (demuxer && demuxer->Aborted())
		return 1;
	return 0;
}

static int dvd_file_read(void *h, uint8_t* buf, int size)
{
	if (interrupt_cb(h))
		return AVERROR_EXIT;

	IInputStream* pInputStream = static_cast<CDemuxFFmpeg*>(h)->m_pInput;
	return pInputStream->Read(buf, size);
}

static int64_t dvd_file_seek(void *h, int64_t pos, int whence)
{
	if (interrupt_cb(h))
		return AVERROR_EXIT;

	IInputStream* pInputStream = static_cast<CDemuxFFmpeg*>(h)->m_pInput;
	if (whence == AVSEEK_SIZE)
		return pInputStream->GetLength();
	else
		return pInputStream->Seek(pos, whence & ~AVSEEK_FORCE);
}