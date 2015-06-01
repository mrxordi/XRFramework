#pragma once

#include "Demux.h"
#include "../XRThreads/CriticalSection.h"
#include "../XRThreads/SystemClock.h"
#include <map>
#include <vector>

extern "C" {
#include "libavformat/avformat.h"
}

class CDVDDemuxFFmpeg;
class CURL;

class CDemuxStreamVideoFFmpeg : public CDemuxStreamVideo
{
	CDVDDemuxFFmpeg *m_parent;
	AVStream*        m_stream;
public:
	CDemuxStreamVideoFFmpeg(CDVDDemuxFFmpeg *parent, AVStream* stream)
		: m_parent(parent)
		, m_stream(stream)
	{}
	virtual void GetStreamInfo(std::string& strInfo);
};


class CDemuxStreamAudioFFmpeg : public CDemuxStreamAudio
{
	CDVDDemuxFFmpeg *m_parent;
	AVStream*        m_stream;
public:
	CDemuxStreamAudioFFmpeg(CDVDDemuxFFmpeg *parent, AVStream* stream)
		: m_parent(parent)
		, m_stream(stream)
	{}
	std::string m_description;

	virtual void GetStreamInfo(std::string& strInfo);
	virtual void GetStreamName(std::string& strInfo);
};

class CDemuxFFmpeg : public CDemux
{
public:
	CDemuxFFmpeg();
	virtual ~CDemuxFFmpeg();

	bool Open(IInputStream* pInput, bool streaminfo = true, bool fileinfo = false);
	void Dispose();
	void Reset();
	void Flush();
	void Abort();
	void SetSpeed(int iSpeed);
	virtual std::string GetFileName();

	DemuxPacket* Read();

	bool SeekTime(int time, bool backwords = false, double* startpts = NULL);
	bool SeekByte(int64_t pos);
	int GetStreamLength();
	CDemuxStream* GetStream(int iStreamId);
	int GetNrOfStreams();

	virtual void GetStreamCodecName(int iStreamId, std::string &strName);

	bool Aborted();

	AVFormatContext* m_pFormatContext;
	IInputStream* m_pInput;

protected:
	friend class CDemuxStreamAudioFFmpeg;
	friend class CDemuxStreamVideoFFmpeg;

	int ReadFrame(AVPacket *packet);
	CDemuxStream* AddStream(int iId);
	void AddStream(int iId, CDemuxStream* stream);
	CDemuxStream* GetStreamInternal(int iStreamId);
	void CreateStreams(unsigned int program = UINT_MAX);
	void DisposeStreams();
	void ParsePacket(AVPacket *pkt);
	bool IsVideoReady();
	void ResetVideoStreams();

	AVDictionary *GetFFMpegOptionsFromURL(const CURL &url);
	double ConvertTimestamp(int64_t pts, int den, int num);
	void UpdateCurrentPTS();
	bool IsProgramChange();

	void GetL16Parameters(int &channels, int &samplerate);

	XR::CCriticalSection m_critSection;
	std::map<int, CDemuxStream*> m_streams;
	std::vector<std::map<int, CDemuxStream*>::iterator> m_stream_index;

	AVIOContext* m_ioContext;

	double   m_currentPts; // used for stream length estimation
	bool     m_bMatroska;
	bool     m_bAVI;
	int      m_speed;
	unsigned m_program;
	XR::EndTime  m_timeout;

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