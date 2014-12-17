#pragma once

//#define _XM_SSE_INTRINSICS_
#include <DirectXMath.h>
#include "RenderModes.h"
#include "TransformMatrix.h"

/* this defines what color translation coefficients */
#define CONF_FLAGS_YUVCOEF_MASK(a) ((a) & 0x07)
#define CONF_FLAGS_YUVCOEF_BT709 0x01
#define CONF_FLAGS_YUVCOEF_BT601 0x02
#define CONF_FLAGS_YUVCOEF_240M  0x03
#define CONF_FLAGS_YUVCOEF_EBU   0x04

#define CONF_FLAGS_YUV_FULLRANGE 0x08
#define CONF_FLAGS_FULLSCREEN    0x10

using namespace DirectX;

void CalculateYUVMatrix(TransformMatrix &matrix
	, unsigned int  flags
	, ERenderFormat format
	, float         black
	, float         contrast);

class YUV2RGBMatrix
{
public:
	YUV2RGBMatrix();
	void SetParameters(float contrast, float blacklevel, unsigned int flags, ERenderFormat format);
	XMFLOAT4X4* Matrix();

private:
	bool         m_NeedRecalc;
	float        m_contrast;
	float        m_blacklevel;
	unsigned int m_flags;
	ERenderFormat m_format;
	XMFLOAT4X4  m_mat;
};