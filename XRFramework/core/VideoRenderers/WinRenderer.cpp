#include "stdafxf.h"
#include "WinRenderer.h"


WinRenderer::WinRenderer()
{
	m_iYV12RenderBuffer = 0;
	m_NumYV12Buffers = 0;
	m_colorShader = nullptr;

	m_requestedMethod = RENDER_AUTO;
	m_renderMethod = RENDER_PS;
	m_scalingMethod = VS_SCALINGMETHOD_LINEAR;

}


WinRenderer::~WinRenderer()
{
}

bool WinRenderer::Configure(UINT width, UINT height, UINT d_width, UINT d_height, float fps, unsigned flags, ERenderFormat format)
{

	return true;
}
