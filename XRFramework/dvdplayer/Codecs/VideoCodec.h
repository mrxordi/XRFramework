#pragma once
#include "core/VideoRenderers/RenderModes.h"

// should be entirely filled by all codecs
struct DVDVideoPicture
{
	double pts; // timestamp in seconds, used in the CDVDPlayer class to keep track of pts
	double dts;

	union
	{
		struct {
			unsigned char* data[4];      // [4] = alpha channel, currently not used
			int iLineSize[4];   // [4] = alpha channel, currently not used
		};
	};

	unsigned int iFlags;

	double       iRepeatPicture;
	double       iDuration;
	unsigned int iFrameType : 4; // see defines above // 1->I, 2->P, 3->B, 0->Undef
	unsigned int color_matrix : 4;
	unsigned int color_range : 1; // 1 indicate if we have a full range of color
	unsigned int chroma_position;
	unsigned int color_primaries;
	unsigned int color_transfer;
	unsigned int extended_format;
	char         stereo_mode[32];

	unsigned char* qp_table; // Quantization parameters, primarily used by filters
	int qstride;
	int qscale_type;

	unsigned int iWidth;
	unsigned int iHeight;
	unsigned int iDisplayWidth;  // width of the picture without black bars
	unsigned int iDisplayHeight; // height of the picture without black bars

	ERenderFormat format;
};

class VideoCodec
{
public:
	VideoCodec();
	~VideoCodec();
};

