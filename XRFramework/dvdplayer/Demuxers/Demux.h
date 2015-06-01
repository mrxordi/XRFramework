#pragma once

class IInputStream;

extern "C" {
#include "libavcodec/avcodec.h"
}

typedef struct DemuxPacket
{
	unsigned char* pData;   // data
	int iSize;     // data size
	int iStreamId; // integer representing the stream index
	int iGroupId;  // the group this data belongs to, used to group data from different streams together

	double pts; // pts in DVD_TIME_BASE
	double dts; // dts in DVD_TIME_BASE
	double duration; // duration in DVD_TIME_BASE if available
} DemuxPacket;

enum StreamType
{
	STREAM_NONE = 0,// if unknown
	STREAM_AUDIO,   // audio stream
	STREAM_VIDEO,   // video stream
};

class CDemuxStream
{
public:
	CDemuxStream()
	{
		iId = 0;
		iPhysicalId = 0;
		codec = (AVCodecID)0; // AV_CODEC_ID_NONE
		codec_fourcc = 0;
		profile = FF_PROFILE_UNKNOWN;
		level = FF_LEVEL_UNKNOWN;
		type = STREAM_NONE;
		iDuration = 0;
		pPrivate = NULL;
		ExtraData = NULL;
		ExtraSize = 0;
		memset(language, 0, sizeof(language));
		disabled = false;
		changes = 0;
		orig_type = 0;
	}

	virtual ~CDemuxStream()
	{
		delete[] ExtraData;
	}

	virtual void GetStreamName(std::string& strInfo);

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

class CDemuxStreamVideo : public CDemuxStream
{
public:
	CDemuxStreamVideo() : CDemuxStream()
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

	virtual ~CDemuxStreamVideo() {}
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
};

class CDemuxStreamAudio : public CDemuxStream
{
public:
	CDemuxStreamAudio() : CDemuxStream()
	{
		iChannels = 0;
		iSampleRate = 0;
		iBlockAlign = 0;
		iBitRate = 0;
		iBitsPerSample = 0;
		type = STREAM_AUDIO;
	}

	virtual ~CDemuxStreamAudio() {}

	void GetStreamType(std::string& strInfo);

	int iChannels;
	int iSampleRate;
	int iBlockAlign;
	int iBitRate;
	int iBitsPerSample;
};

class CDemux
{
public:

	CDemux() {}
	virtual ~CDemux() {}

	/*
	* Reset the entire demuxer (same result as closing and opening it)
	*/
	virtual void Reset() = 0;

	/*
	* Aborts any internal reading that might be stalling main thread
	* NOTICE - this can be called from another thread
	*/
	virtual void Abort() = 0;

	/*
	* Flush the demuxer, if any data is kept in buffers, this should be freed now
	*/
	virtual void Flush() = 0;

	/*
	* Read a packet, returns NULL on error
	*
	*/
	virtual DemuxPacket* Read() = 0;

	/*
	* Seek, time in msec calculated from stream start
	*/
	virtual bool SeekTime(int time, bool backwords = false, double* startpts = NULL) = 0;


	/*
	* Set the playspeed, if demuxer can handle different
	* speeds of playback
	*/
	virtual void SetSpeed(int iSpeed) = 0;

	/*
	* returns the total time in msec
	*/
	virtual int GetStreamLength() = 0;

	/*
	* returns the stream or NULL on error, starting from 0
	*/
	virtual CDemuxStream* GetStream(int iStreamId) = 0;

	/*
	* return nr of streams, 0 if none
	*/
	virtual int GetNrOfStreams() = 0;

	/*
	* returns opened filename
	*/
	virtual std::string GetFileName() = 0;
	/*
	* return nr of audio streams, 0 if none
	*/
	int GetNrOfAudioStreams();

	/*
	* return nr of video streams, 0 if none
	*/
	int GetNrOfVideoStreams();

	/*
	* return the audio stream, or NULL if it does not exist
	*/
	CDemuxStreamAudio* GetStreamFromAudioId(int iAudioIndex);

	/*
	* return the video stream, or NULL if it does not exist
	*/
	CDemuxStreamVideo* GetStreamFromVideoId(int iVideoIndex);

	/*
	* return a user-presentable codec name of the given stream
	*/
	virtual void GetStreamCodecName(int iStreamId, std::string &strName) {};
};