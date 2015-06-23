#pragma once
#include <memory>

#include "YUV2RGBShader.h"
#include "CDX10Shader.h"
#include "RenderModes.h"
#include "wx/window.h"
#include "utils/MyEvents.h"
//#include "dvdplayer/Codecs/VideoCodec.h"
#include "XRThreads/CriticalSection.h"
#include "XRThreads/SingleLock.h"

#define MAX_PLANES 3
#define MAX_FIELDS 3
#define NUM_BUFFERS 6
#define AUTOSOURCE -1

class CDX10SystemRenderer;
struct Context;
class wxDX10Display;

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
	for (unsigned i = 0; RenderMethodDetails[i].method != RENDER_INVALID; i++) 
	{
		if (RenderMethodDetails[i].method == m)
			return &RenderMethodDetails[i];
	}
	return NULL;
}

class CDX10FrameRenderer
{
public:
	CDX10FrameRenderer(Context* context);
	virtual ~CDX10FrameRenderer();

	bool Configure(UINT width, UINT height, float fps, unsigned int flags, ERenderFormat format, wxDX10Display* window);
	void Release();
	void Render();
	void RenderPS();
   void Flip(int source);
	
	void CalculateFrameAspectRatio(unsigned int desired_width, unsigned int desired_height);
	void ManageDisplay();
	void ManageTextures();
	
//	int AddVideoPicture(DVDVideoPicture& pic);
   int GetImage(YV12Image* image, int source = AUTOSOURCE);
	void ReleaseImage(int index);
	
	int NextYV12Texture();
	void DeleteYV12Texture(int index);
	bool CreateYV12Texture(int index);

	void UpdatePSVideoFilter();

	void SetBufferSize(int size) { m_neededBuffers = size; }
	int  GetMaxBufferSize() { return NUM_BUFFERS; }

	XR::CCriticalSection m_eventlock;
protected:
	void Stage1();
	void Stage2();

	void SendVideoRendererMessage(wxVideoRendererEvent::VR_ACTION action);
private:
	Context *m_context;
	int  m_iYV12RenderBuffer;
	int  m_NumYV12Buffers;
	int  m_neededBuffers;

	//D3DTexture			m_IntermediateTarget;
	bool				m_bUseHQScaler;
	bool                m_bConfigured;
	bool				 m_bFiltersInitialized;

	unsigned int		m_iFlags;
	int					m_requestedMethod;
	YUVBuffer	      *m_VideoBuffers[NUM_BUFFERS];
	float				   m_fps;
	ERenderFormat		m_format;
	YUV2RGBShader*		m_colorShader;
	ERenderMethod		m_renderMethod;
	EScalingMethod		m_scalingMethod;
	

	// clear colour for "black" bars
	DWORD               m_clearColour;

	unsigned int m_sourceWidth;
	unsigned int m_sourceHeight;
	float        m_sourceFrameRatio;

	// Width and height of the render target
	unsigned int         m_destWidth;
	unsigned int         m_destHeight;

	XRect				m_sourceRect;
	XRect				m_destRect;
	XRect				m_olddestRest;


	unsigned int         m_frameIdx;

	XR::CCriticalSection m_sharedSection;
	XR::CCriticalSection m_presentLock;
};

