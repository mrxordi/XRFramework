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
	RenderMethods  method;
	const char   *name;
} RenderMethodDetail;

static RenderMethodDetail RenderMethodDetails[] = {
		{ RENDER_METHOD_SW, "Software" },
		{ RENDER_METHOD_D3D_PS, "Pixel Shaders" },
		{ RENDER_METHOD_AUTO, "AUTO" },
};

class WinRenderer
{
public:
	WinRenderer();
	~WinRenderer();

private:
	int  m_iYV12RenderBuffer;
	int  m_NumYV12Buffers;

	bool                 m_bConfigured;
	YUVBuffer	        *m_VideoBuffers[NUM_BUFFERS];
};

