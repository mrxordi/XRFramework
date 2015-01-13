#pragma once
#include "YUV2RGBShader.h"
#include "WinShader.h"
#include "RenderModes.h"

#define MAX_PLANES 3
#define MAX_FIELDS 3
#define NUM_BUFFERS 6

typedef struct YV12Image
{
	uint8_t* plane[MAX_PLANES];
	int      planesize[MAX_PLANES];
	unsigned stride[MAX_PLANES];
	unsigned width;
	unsigned height;
	unsigned flags;

	unsigned cshift_x; /* this is the chroma shift used */
	unsigned cshift_y;
	unsigned bpp; /* bytes per pixel */
} YV12Image;

typedef struct {
	ERenderMethod  method;
	const char   *name;
} RenderMethodDetail;

static RenderMethodDetail RenderMethodDetails[] = {
		{ RENDER_SW, "Software" },
		{ RENDER_PS, "Pixel Shaders" },
		{ RENDER_INVALID, NULL },
};

static RenderMethodDetail *FindRenderMethod(ERenderMethod m)
{
	for (unsigned i = 0; RenderMethodDetails[i].method != RENDER_INVALID; i++) {
		if (RenderMethodDetails[i].method == m)
			return &RenderMethodDetails[i];
	}
	return NULL;
}

class WinRenderer
{
public:
	WinRenderer();
	~WinRenderer();

	bool Configure(UINT width, UINT height, UINT d_width, UINT d_height, float fps, unsigned flags, ERenderFormat format);



private:
	int  m_iYV12RenderBuffer;
	int  m_NumYV12Buffers;

	D3DTexture			m_IntermediateTarget;

	bool                 m_bConfigured;
	YUVBuffer	        *m_VideoBuffers[NUM_BUFFERS];
	float				m_fps;
	ERenderFormat		m_format;
	YUV2RGBShader*		m_colorShader;
	int					m_requestedMethod;
	ERenderMethod		m_renderMethod;
	EScalingMethod		m_scalingMethod;
	

	// clear colour for "black" bars
	DWORD               m_clearColour;

	// Width and height of the render target
	// the separable HQ scalers need this info, but could the m_destRect be used instead?
	unsigned int         m_destWidth;
	unsigned int         m_destHeight;


	int                  m_neededBuffers;
	unsigned int         m_frameIdx;
};

