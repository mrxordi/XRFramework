#include "stdafxf.h"
#include "DX10FrameRenderer.h"
#include <algorithm>
#include <cmath>
#include "Base.h"
#include "../XRFwx/main.h"
#include "TestShader.h"
#include "YUV2RGBShader.h"
#include "wx/wx.h"
//#include "dvdplayer/Codecs/VideoCodec.h"
#include "XRCommon/settings/AppSettings.h"
#include "render/RenderSystemDX.h"
#include "XRFwx/Context.h"



CDX10FrameRenderer::CDX10FrameRenderer(Context* ctx) : m_context(ctx)
{
	m_iYV12RenderBuffer = 0;
	m_NumYV12Buffers = 0;
	m_neededBuffers = 3;

	m_colorShader = nullptr;

	m_requestedMethod = RENDER_AUTO;
	m_renderMethod = RENDER_PS;
	m_scalingMethod = VS_SCALINGMETHOD_LINEAR;
	for (int i = 0; i < NUM_BUFFERS; i++)
		m_VideoBuffers[i] = nullptr;

	m_bFiltersInitialized = false;
	m_format = RENDER_FMT_NONE;
	m_sourceWidth = m_sourceHeight = 0;
	m_sourceFrameRatio = 0.0;
	m_fps = 0.0f;
	m_bUseHQScaler = false;
   m_bConfigured = false;
}

CDX10FrameRenderer::~CDX10FrameRenderer() 
{
	//We'll use this after well make sure that destructor will be not called from wxApp destructor
	//as destructor stops sending messages first 
	//SendVideoRendererMessage(wxVideoRendererEvent::VR_ACTION_DETACH);
	SAFE_DELETE(m_colorShader);

	for (int i = 0; i < NUM_BUFFERS; ++i)
		DeleteYV12Texture(i);

	LOGDEBUG("Wideo Renderer now is destroyed.");
}

bool CDX10FrameRenderer::Configure(UINT width, UINT height, float fps, unsigned int flags, ERenderFormat format, wxDX10Display* window) 
{
	if (m_bConfigured)
		Release();

	//If no window pointer provided we check that are we reconfiguring renderer
	//if yes then we check is the window the same or when is null we use previosly saved window ptr.
	if ((!window && !m_context->videoDisplay) && (m_context->videoDisplay && m_context->videoDisplay != window))
	{
		LOGFATAL("No window provided or configuring for different window (MUST RECREATE RENDERER!)");
		return false;
	}

	if (m_sourceWidth != width || m_sourceHeight != height || m_format != format)
	{
		m_sourceWidth = width;
		m_sourceHeight = height;
		// need to recreate textures
		m_NumYV12Buffers = 0;
		m_iYV12RenderBuffer = 0;
		// reinitialize the filters/shaders
		m_bFiltersInitialized = false;
	} else 
	{
		if (m_VideoBuffers[m_iYV12RenderBuffer] != NULL)
			m_VideoBuffers[m_iYV12RenderBuffer]->StartDecode();

		m_iYV12RenderBuffer = 0;
		if (m_VideoBuffers[0] != NULL)
			m_VideoBuffers[0]->StartRender();
	}
	m_fps = fps;
	m_format = format;
	m_iFlags = flags;
	CalculateFrameAspectRatio(width, height);

	{
		TestShader shader(m_context->videoDisplay->m_pDXSystemRenderer.get());
		if (!shader.Create()) 
		{
			LOGERR("D3D: unable to load test shader - D3D installation is most likely incomplete!");
			return false;
		}
	}

	ManageDisplay();

	LOGDEBUG("Video Renderer Configured - video: %ux%u@%f fps, frameratio:%f", width, height, fps, m_sourceFrameRatio);
	LOGDEBUG("Video Renderer current source rect: %lu-%lu x %lu-%lu ", m_sourceRect.left, m_sourceRect.right, m_sourceRect.top, m_sourceRect.bottom);
	LOGDEBUG("Video Renderer current target rect: %lu-%lu x %lu-%lu ", m_destRect.left, m_destRect.right, m_destRect.top, m_destRect.bottom);

// 	if (!m_bConfigured)
// 		SendVideoRendererMessage(wxVideoRendererEvent::VR_ACTION_ATTACH);

	m_bConfigured = true;
	return true;
}

void CDX10FrameRenderer::Flip(int source)
{
   if (source == AUTOSOURCE)
      source = NextYV12Texture();

   if (m_VideoBuffers[m_iYV12RenderBuffer] != NULL)
      m_VideoBuffers[m_iYV12RenderBuffer]->StartDecode();

   if (source >= 0 && source < m_NumYV12Buffers)
      m_iYV12RenderBuffer = source;
   else
      m_iYV12RenderBuffer = 0;

   if (m_VideoBuffers[m_iYV12RenderBuffer] != NULL)
      m_VideoBuffers[m_iYV12RenderBuffer]->StartRender();
}


void CDX10FrameRenderer::CalculateFrameAspectRatio(unsigned int desired_width, unsigned int desired_height) 
{

	m_sourceFrameRatio = (float)desired_width / desired_height;

	// Check whether mplayer has decided that the size of the video file should be changed
	// This indicates either a scaling has taken place (which we didn't ask for) or it has
	// found an aspect ratio parameter from the file, and is changing the frame size based
	// on that.
	if (m_sourceWidth == (unsigned int)desired_width && m_sourceHeight == (unsigned int)desired_height)
		return;

	// mplayer is scaling in one or both directions.  We must alter our Source Pixel Ratio
	float imageFrameRatio = (float)m_sourceWidth / m_sourceHeight;

	// OK, most sources will be correct now, except those that are intended
	// to be displayed on non-square pixel based output devices (ie PAL or NTSC TVs)
	// This includes VCD, SVCD, and DVD (and possibly others that we are not doing yet)
	// For this, we can base the pixel ratio on the pixel ratios of PAL and NTSC,
	// though we will need to adjust for anamorphic sources (ie those whose
	// output frame ratio is not 4:3) and for SVCDs which have 2/3rds the
	// horizontal resolution of the default NTSC or PAL frame sizes

	// The following are the defined standard ratios for PAL and NTSC pixels
	// NOTE: These aren't technically (in terms of BT601) correct - the commented values are,
	//       but it seems that many DVDs nowadays are mastered incorrectly, so two wrongs
	//       may indeed make a right.  The "wrong" values here ensure the output frame is
	//       4x3 (or 16x9)
	const float PALPixelRatio = 16.0f / 15.0f;      // 128.0f / 117.0f;
	const float NTSCPixelRatio = 8.0f / 9.0f;       // 4320.0f / 4739.0f;

	// Calculate the correction needed for anamorphic sources
	float Non4by3Correction = m_sourceFrameRatio / (4.0f / 3.0f);

	// Finally, check for a VCD, SVCD or DVD frame size as these need special aspect ratios
	if (m_sourceWidth == 352) 
	{ // VCD?
		if (m_sourceHeight == 240) // NTSC
			m_sourceFrameRatio = imageFrameRatio * NTSCPixelRatio;
		if (m_sourceHeight == 288) // PAL
			m_sourceFrameRatio = imageFrameRatio * PALPixelRatio;
	}
	if (m_sourceWidth == 480) 
	{ // SVCD?
		if (m_sourceHeight == 480) // NTSC
			m_sourceFrameRatio = imageFrameRatio * 3.0f / 2.0f * NTSCPixelRatio * Non4by3Correction;
		if (m_sourceHeight == 576) // PAL
			m_sourceFrameRatio = imageFrameRatio * 3.0f / 2.0f * PALPixelRatio * Non4by3Correction;
	}
	if (m_sourceWidth == 720) 
	{ // DVD?
		if (m_sourceHeight == 480) // NTSC
			m_sourceFrameRatio = imageFrameRatio * NTSCPixelRatio * Non4by3Correction;
		if (m_sourceHeight == 576) // PAL
			m_sourceFrameRatio = imageFrameRatio * PALPixelRatio * Non4by3Correction;
	}
}

int CDX10FrameRenderer::GetImage(YV12Image* image, int source) 
{
	/* take next available buffer */
	if (source == AUTOSOURCE)
		source = NextYV12Texture();

	if (source < 0 || NextYV12Texture() < 0)
		return -1;

	YUVBuffer *buf = m_VideoBuffers[source];

	if (buf->IsReadyToRender())
		return -1;

	image->cshift_x = 1;
	image->cshift_y = 1;
	image->height = m_sourceHeight;
	image->width = m_sourceWidth;
	image->flags = 0;
	if (m_format == RENDER_FMT_YUV420P10
		|| m_format == RENDER_FMT_YUV420P16)
		image->bpp = 2;
	else
		image->bpp = 1;

	for (int i = 0; i < 3; i++) 
	{
		image->stride[i] = buf->planes[i].rect.RowPitch;
		image->plane[i] = (BYTE*)buf->planes[i].rect.pData;
	}
	return source;
}

void CDX10FrameRenderer::ReleaseImage(int index)
{
}

void CDX10FrameRenderer::ManageTextures() 
{

	if (m_NumYV12Buffers < m_neededBuffers) 
	{
		for (int i = m_NumYV12Buffers; i < m_neededBuffers; i++)
			CreateYV12Texture(i);

		m_NumYV12Buffers = m_neededBuffers;
	}
	else if (m_NumYV12Buffers > m_neededBuffers) 
	{
		m_NumYV12Buffers = m_neededBuffers;
		m_iYV12RenderBuffer = m_iYV12RenderBuffer % m_NumYV12Buffers;

		for (int i = m_NumYV12Buffers - 1; i >= m_neededBuffers; i--)
			DeleteYV12Texture(i);
	}
}

int CDX10FrameRenderer::NextYV12Texture()
{
	if (m_NumYV12Buffers)
		return (m_iYV12RenderBuffer + 1) % m_NumYV12Buffers;
	else
		return -1;
}

void CDX10FrameRenderer::DeleteYV12Texture(int index) 
{

	if (m_VideoBuffers[index] != NULL) 
	{
		LOGDEBUG(" - Deleted video buffer %i", index);
		SAFE_DELETE(m_VideoBuffers[index]);
	}
		m_NumYV12Buffers = 0;
}

bool CDX10FrameRenderer::CreateYV12Texture(int index)
{
	DeleteYV12Texture(index);

	YUVBuffer* buf = new YUVBuffer(m_context->videoDisplay->m_pDXSystemRenderer.get());

	if (!buf->Create(m_format, m_sourceWidth, m_sourceHeight)) 
	{
		LOGERR(" - Unable to create YV12 video texture %i", index);
		return false;
	}
	m_VideoBuffers[index] = buf;

	buf->StartDecode();
	buf->Clear();

	LOGDEBUG(" - Created video buffer %i", index);
	return true;
}

void CDX10FrameRenderer::UpdatePSVideoFilter() 
{
	if (m_bFiltersInitialized)
		return;

	if (m_bUseHQScaler)
		throw;
	
	SAFE_DELETE(m_colorShader);

	if (!m_bUseHQScaler) 
	{
		m_colorShader = new YUV2RGBShader(m_context->videoDisplay->m_pDXSystemRenderer.get());
		if (!m_colorShader->Create(m_sourceWidth, m_sourceHeight, m_format)) 
		{
			SAFE_DELETE(m_colorShader);
			LOGFATAL("Failed to create YUV color shader!");
			m_bFiltersInitialized = false;
		}
	}
	m_bFiltersInitialized = true;
}

void CDX10FrameRenderer::Render()
{
	if (!m_bConfigured)
		return;
	ManageDisplay();
	UpdatePSVideoFilter();
	ManageTextures();

	if (m_iYV12RenderBuffer < 0 || !m_VideoBuffers[m_iYV12RenderBuffer]->IsReadyToRender())
		return;

	RenderPS();
}

void CDX10FrameRenderer::RenderPS()
{
	if (!m_bUseHQScaler) 
	{
		Stage1();
	} else 
	{
		Stage1();
		Stage2();
	}
}

void CDX10FrameRenderer::Stage1()
{
	if (!m_bUseHQScaler) 
	{
		m_colorShader->Render(m_sourceRect, m_destRect, g_AppSettings->m_contrast, g_AppSettings->m_brightness, m_iFlags, m_VideoBuffers[m_iYV12RenderBuffer]);
	}
	else {
		//NOT IMPLEMENTED
	}
}

void CDX10FrameRenderer::Stage2()
{
	//NOT IMPLEMENTED
}

void CDX10FrameRenderer::ManageDisplay() 
{
	const wxRect rect = m_context->videoDisplay->GetRect();
	const XRect view(0, 0, rect.GetWidth(), rect.GetHeight());

	m_sourceRect.left = 0.0f;
	m_sourceRect.top = 0.0f;
	m_sourceRect.right = (float)m_sourceWidth;
	m_sourceRect.bottom = (float)m_sourceHeight;

	float newWidth = view.Width();
	float newHeight = newWidth / m_sourceFrameRatio;

	if (newHeight > view.Height()) 
	{
		newHeight = view.Height();
		newWidth = newHeight * m_sourceFrameRatio;
	}

	// if we are less than one pixel off use the complete screen instead
	if (std::abs(newWidth - view.Width()) < 1.0f)
		newWidth = view.Width();
	if (std::abs(newHeight - view.Height()) < 1.0f)
		newHeight = view.Height();

	float posX = (view.Width() - newWidth) / 2;
	float posY = (view.Height() - newHeight) / 2;

	m_destRect.left = (float)std::rint(posX);
	m_destRect.right = m_destRect.left + std::rint(newWidth);
	m_destRect.top = (float)std::rint(posY);
	m_destRect.bottom = m_destRect.top + std::rint(newHeight);
}

// int CDX10FrameRenderer::AddVideoPicture(DVDVideoPicture& pic)
// {
// 	return 1;
// }

void CDX10FrameRenderer::SendVideoRendererMessage(wxVideoRendererEvent::VR_ACTION action) 
{

	//We have to make sure that Event has been processed before next event can be send
	XR::CSingleLock lock(m_eventlock);
	wxVideoRendererEvent ev = wxVideoRendererEvent(this, action);
	m_context->videoDisplay->OnVideoRendererEvent(ev);
}

void CDX10FrameRenderer::Release() 
{
	//We call this function whenever contexted window is destroying, or 
	//while creating new DXrendering system (for example fullscreen toggling)
	m_NumYV12Buffers = 0;
	m_iYV12RenderBuffer = -1;
	// reinitialize the filters/shaders
	m_bFiltersInitialized = false;

	SAFE_DELETE(m_colorShader);

	for (int i = 0; i < NUM_BUFFERS; ++i)
		DeleteYV12Texture(i);

	if (m_bConfigured)
		SendVideoRendererMessage(wxVideoRendererEvent::VR_ACTION_DETACH);

	m_bConfigured = false;
}
