#include "stdafxf.h"
#include "YUV2RGBShader.h"
#include "render/RenderSystemDX.h"


/************************************************************************/
/*    YUVBuffer                                                         */
/************************************************************************/

YUVBuffer::YUVBuffer() : m_width(0), m_height(0), m_format(RENDER_FMT_NONE), m_activeplanes(0), m_locked(false)
{
	for (int i = 0; i > MAX_PLANES; ++i)
		planes[i].texture = std::make_unique<D3DTexture>();
}

YUVBuffer::~YUVBuffer()
{
	if (planes[0].texture->GetResource() != NULL)
		Release();
}

bool YUVBuffer::Create(ERenderFormat format, unsigned int width, unsigned int height)
{
	m_format = format;
	m_width = width;
	m_height = height;

	switch (m_format) {
	case RENDER_FMT_YUV420P: {
		if (!planes[PLANE_Y].texture->Create(m_width, m_height, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DYNAMIC, DXGI_FORMAT_R8_UNORM)
			|| !planes[PLANE_U].texture->Create(m_width / 2, m_height / 2, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DYNAMIC, DXGI_FORMAT_R8_UNORM)
			|| !planes[PLANE_V].texture->Create(m_width / 2, m_height / 2, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DYNAMIC, DXGI_FORMAT_R8_UNORM))
			return false;
		m_activeplanes = 3;
		break;
	}
	case RENDER_FMT_YUV420P10:
	case RENDER_FMT_YUV420P16: {
		if (!planes[PLANE_Y].texture->Create(m_width, m_height, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DYNAMIC, DXGI_FORMAT_R16_UNORM)
			|| !planes[PLANE_U].texture->Create(m_width / 2, m_height / 2, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DYNAMIC, DXGI_FORMAT_R16_UNORM)
			|| !planes[PLANE_V].texture->Create(m_width / 2, m_height / 2, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DYNAMIC, DXGI_FORMAT_R16_UNORM))
				return false;
			m_activeplanes = 3;
			break;
		}	
	case RENDER_FMT_NV12: {
		if (!planes[PLANE_Y].texture->Create(m_width, m_height, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DYNAMIC, DXGI_FORMAT_R8_UNORM)
			|| !planes[PLANE_UV].texture->Create(m_width / 2, m_height / 2, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DYNAMIC, DXGI_FORMAT_R8G8_UNORM))
			return false;
		m_activeplanes = 2;
		break;
	}
	case RENDER_FMT_YUYV422: {
		if (!planes[PLANE_Y].texture->Create(m_width >> 1, m_height, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DYNAMIC, DXGI_FORMAT_B8G8R8A8_UNORM))
			return false;
		m_activeplanes = 1;
		break;
	}
	case RENDER_FMT_UYVY422: {
		if (!planes[PLANE_Y].texture->Create(m_width >> 1, m_height, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DYNAMIC, DXGI_FORMAT_B8G8R8A8_UNORM))
			return false;
		m_activeplanes = 1;
		break;
	}
	default:
		m_activeplanes = 0;
		return false;
	}

	return true;
}

void YUVBuffer::Release()
{
	if (planes[0].texture->GetResource() != 0)
		for (unsigned i = 0; i < m_activeplanes; i++)
		{
		planes[i].texture->Release();
		memset(&planes[i].rect, 0, sizeof(planes[i].rect));
		}

	m_activeplanes = 0;
}

void YUVBuffer::StartDecode()
{
	if (m_locked)
		return;

	m_locked = true;

	for (unsigned i = 0; i < m_activeplanes; i++)
	{
		if (planes[i].texture->GetResource()
			&& planes[i].texture->Lock(0, D3D10_MAP_WRITE_DISCARD, &planes[i].rect) == false)
		{
			memset(&planes[i].rect, 0, sizeof(planes[i].rect));
			LOGERR(" - failed to lock texture %d into memory", i);
		}
	}
}

void YUVBuffer::StartRender()
{
	if (!m_locked)
		return;

	m_locked = false;

	for (unsigned i = 0; i < m_activeplanes; i++)
	{
		if (planes[i].texture->Get() && planes[i].rect.pData)
			if (!planes[i].texture->Unlock(0))
				LOGERR(" - failed to unlock texture %d", i);
		memset(&planes[i].rect, 0, sizeof(planes[i].rect));
	}
}

void YUVBuffer::Clear()
{
	// Set Y to 0 and U,V to 128 (RGB 0,0,0) to avoid visual artifacts at the start of playback
	switch (m_format)
	{
	case RENDER_FMT_YUV420P16:
	{
		wmemset((wchar_t*)planes[PLANE_Y].rect.pData, 0, planes[PLANE_Y].rect.RowPitch *  m_height / 2);
		wmemset((wchar_t*)planes[PLANE_U].rect.pData, 32768, planes[PLANE_U].rect.RowPitch * (m_height / 2) / 2);
		wmemset((wchar_t*)planes[PLANE_V].rect.pData, 32768, planes[PLANE_V].rect.RowPitch * (m_height / 2) / 2);
		break;
	}
	case RENDER_FMT_YUV420P10:
	{
		wmemset((wchar_t*)planes[PLANE_Y].rect.pData, 0, planes[PLANE_Y].rect.RowPitch *  m_height / 2);
		wmemset((wchar_t*)planes[PLANE_U].rect.pData, 512, planes[PLANE_U].rect.RowPitch * (m_height / 2) / 2);
		wmemset((wchar_t*)planes[PLANE_V].rect.pData, 512, planes[PLANE_V].rect.RowPitch * (m_height / 2) / 2);
		break;
	}
	case RENDER_FMT_YUV420P:
	{
		memset(planes[PLANE_Y].rect.pData, 0, planes[PLANE_Y].rect.RowPitch *  m_height);
		memset(planes[PLANE_U].rect.pData, 128, planes[PLANE_U].rect.RowPitch * (m_height / 2));
		memset(planes[PLANE_V].rect.pData, 128, planes[PLANE_V].rect.RowPitch * (m_height / 2));
		break;
	}
	case RENDER_FMT_NV12:
	{
		memset(planes[PLANE_Y].rect.pData, 0, planes[PLANE_Y].rect.RowPitch *  m_height);
		memset(planes[PLANE_UV].rect.pData, 128, planes[PLANE_U].rect.RowPitch * (m_height / 2));
		break;
	}
		// YUY2, UYVY: wmemset to set a 16bit pattern, byte-swapped because x86 is LE
	case RENDER_FMT_YUYV422:
	{
		wmemset((wchar_t*)planes[PLANE_Y].rect.pData, 0x8000, planes[PLANE_Y].rect.RowPitch / 2 * m_height);
		break;
	}
	case RENDER_FMT_UYVY422:
	{
		wmemset((wchar_t*)planes[PLANE_Y].rect.pData, 0x0080, planes[PLANE_Y].rect.RowPitch / 2 * m_height);
		break;
	}

	}
}

bool YUVBuffer::IsReadyToRender()
{
	if (!m_locked)
		return true;
	return false;
}


//###############################################
//#  YUV2RGBShader
//###############################################
YUV2RGBShader::~YUV2RGBShader()
{
	for (unsigned i = 0; i < MAX_PLANES; i++)
	{
		if (m_YUVPlanes[i].Get() || m_YUVPlanes[i].GetResource())
			m_YUVPlanes[i].Release();
	}
}

bool YUV2RGBShader::Create(UINT srcWidth, UINT srcHeight, ERenderFormat format)
{
	if (!CreateVertexBuffer(4, sizeof(CUSTOMVERTEX), 1)) {
		LOGFATAL("Failed to create Vertex Buffer. (No memory?)");
		return false;
	}

	if (!CreateIndexBuffer(5)) {
		LOGFATAL("Failed to create Vertex Buffer. (No memory?)");
		return false;
	}

	m_sourceWidth = srcWidth;
	m_sourceHeight = srcHeight;
	m_format = format;

	unsigned int texWidth;

	DefinesMap defines;

	if (format == RENDER_FMT_YUV420P16 || format == RENDER_FMT_YUV420P10)
	{
		defines["XBMC_YV12"] = "";
		texWidth = srcWidth;

		if (!m_YUVPlanes[0].Create(texWidth, m_sourceHeight, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DEFAULT, DXGI_FORMAT_R16_UNORM)
			|| !m_YUVPlanes[1].Create(texWidth / 2, m_sourceHeight / 2, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DEFAULT, DXGI_FORMAT_R16_UNORM)
			|| !m_YUVPlanes[2].Create(texWidth / 2, m_sourceHeight / 2, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DEFAULT, DXGI_FORMAT_R16_UNORM))
		{
			LOGERR(": Failed to create 16 bit YV12 planes.");
			return false;
		}
	}
	else if (format == RENDER_FMT_YUV420P)
	{
		defines["XBMC_YV12"] = "";
		texWidth = srcWidth;

		if (!m_YUVPlanes[0].Create(texWidth, m_sourceHeight, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DEFAULT, DXGI_FORMAT_R8_UNORM)
			|| !m_YUVPlanes[1].Create(texWidth / 2, m_sourceHeight / 2, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DEFAULT, DXGI_FORMAT_R8_UNORM)
			|| !m_YUVPlanes[2].Create(texWidth / 2, m_sourceHeight / 2, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DEFAULT, DXGI_FORMAT_R8_UNORM))
		{
			LOGERR(": Failed to create YV12 planes.");
			return false;
		}
	}
	else if (format == RENDER_FMT_NV12)
	{
		defines["XBMC_NV12"] = "";
		texWidth = srcWidth;

		if (!m_YUVPlanes[0].Create(texWidth, m_sourceHeight, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DEFAULT, DXGI_FORMAT_R8_UNORM)
			|| !m_YUVPlanes[1].Create(texWidth / 2, m_sourceHeight / 2, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DEFAULT, DXGI_FORMAT_R8G8_UNORM))
		{
			LOGERR(": Failed to create NV12 planes.");
			return false;
		}
	}
	else if (format == RENDER_FMT_YUYV422)
	{
		defines["XBMC_YUY2"] = "";
		texWidth = srcWidth >> 1;

		if (!m_YUVPlanes[0].Create(texWidth, m_sourceHeight, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DEFAULT, DXGI_FORMAT_B8G8R8A8_UNORM))
		{
			LOGERR(": Failed to create YUY2 planes.");
			return false;
		}
	}
	else if (format == RENDER_FMT_UYVY422)
	{
		defines["XBMC_UYVY"] = "";
		texWidth = srcWidth >> 1;

		if (!m_YUVPlanes[0].Create(texWidth, m_sourceHeight, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DEFAULT, DXGI_FORMAT_B8G8R8A8_UNORM))
		{
			LOGERR(": Failed to create UYVY planes.");
			return false;
		}
	}
	else
		return false;

	m_texSteps[0] = 1.0f / (float)texWidth;
	m_texSteps[1] = 1.0f / (float)srcHeight;

	if (!LoadEffect("special://app/system/yuv2rgb_d3d.fx", &defines)) {
		m_effect.Release();
		return false;
	}

	m_effect.SetTechnique("YUV2RGB");
	BuildVertexLayout();
	return true;
}

void YUV2RGBShader::Render(XRect sourceRect, XRect destRect, float contrast, float brightness, unsigned int flags, YUVBuffer* YUVbuf)
{
	PrepareParameters(sourceRect, destRect, contrast, brightness, flags);
	UploadToGPU(YUVbuf);
	SetShaderParameters(YUVbuf);
	Execute(NULL, 0);
}

void YUV2RGBShader::PrepareParameters(XRect sourceRect, XRect destRect, float contrast, float brightness, unsigned int flags)
{
	//See RGB renderer for comment on this
#define CHROMAOFFSET_HORIZ 0.25f

	if (m_sourceRect != sourceRect || m_destRect != destRect)
	{
		m_sourceRect = sourceRect;
		m_destRect = destRect;

		CUSTOMVERTEX* v;
		LockVertexBuffer((void**)&v);

		//left top corner of destination
		v[0].coord.x = destRect.left;
		v[0].coord.y = destRect.top;
		//left top corner of source UAV surfaces
		v[0].tyv.x = sourceRect.left / m_sourceWidth;
		v[0].tyv.y = sourceRect.top / m_sourceHeight;
		v[0].tuv.x = v[0].tvv.x = (sourceRect.left / 2.0f + CHROMAOFFSET_HORIZ) / (m_sourceWidth >> 1);
		v[0].tuv.y = v[0].tvv.y = (sourceRect.top / 2.0f + CHROMAOFFSET_HORIZ) / (m_sourceHeight >> 1);

		//right top corner of destination
		v[1].coord.x = destRect.right;
		v[1].coord.y = destRect.top;
		//right top corner of source UAV surfaces
		v[1].tyv.x = sourceRect.Width() / m_sourceWidth;
		v[1].tyv.y = sourceRect.top / m_sourceHeight;
		v[1].tuv.x = v[1].tvv.x = (sourceRect.Width() / 2.0f + CHROMAOFFSET_HORIZ) / (m_sourceWidth >> 1);
		v[1].tuv.y = v[1].tvv.y = (sourceRect.top / 2.0f + CHROMAOFFSET_HORIZ) / (m_sourceHeight >> 1);

		//right bottom corner of destination
		v[2].coord.x = destRect.right;
		v[2].coord.y = destRect.bottom;
		//right bottom corner of source UAV surfaces
		v[2].tyv.x = sourceRect.Width() / m_sourceWidth;
		v[2].tyv.y = sourceRect.Height() / m_sourceHeight;
		v[2].tuv.x = v[2].tvv.x = (sourceRect.Width() / 2.0f + CHROMAOFFSET_HORIZ) / (m_sourceWidth >> 1);
		v[2].tuv.y = v[2].tvv.y = (sourceRect.Height() / 2.0f + CHROMAOFFSET_HORIZ) / (m_sourceHeight >> 1);

		//left bottom corner of destination
		v[3].coord.x = destRect.left;
		v[3].coord.y = destRect.bottom;
		//left bottom corner of source UAV surfaces
		v[3].tyv.x = sourceRect.left / m_sourceWidth;
		v[3].tyv.y = sourceRect.Height() / m_sourceHeight;
		v[3].tuv.x = v[3].tvv.x = (sourceRect.left / 2.0f + CHROMAOFFSET_HORIZ) / (m_sourceWidth >> 1);
		v[3].tuv.y = v[3].tvv.y = (sourceRect.Height() / 2.0f + CHROMAOFFSET_HORIZ) / (m_sourceHeight >> 1);

		UINT f = (m_sourceHeight >> 1);
		// -0.5 offset to compensate for D3D rasterization
		// set z and rhw
		for (int i = 0; i < 4; i++)
		{
			v[i].coord.x -= 0.5;
			v[i].coord.y -= 0.5;
			v[i].coord.z = 0.0f;
			v[i].coord.w = 1.0f;
		}
		UnlockVertexBuffer();
	}

	int* f;
	if (LockIndexBuffer((void**)&f)) {
		f[0] = 2;
		f[1] = 3;
		f[2] = 0;
		f[3] = 1;
		f[4] = 2;

		UnlockIndexBuffer();
	}

	m_matrix.SetParameters(contrast * 0.02f, brightness * 0.01f - 0.5f, flags, m_format);
}

void YUV2RGBShader::SetShaderParameters(YUVBuffer* YUVbuf)
{
	m_effect.SetMatrix("gW", g_DXRendererPtr->m_world);
	m_effect.SetMatrix("gV", g_DXRendererPtr->m_view);
	m_effect.SetMatrix("gP", g_DXRendererPtr->m_projection);
	m_effect.SetMatrix("g_ColorMatrix", *m_matrix.Matrix());
	m_effect.SetTexture("g_YTexture", m_YUVPlanes[0]);
	if (YUVbuf->GetActivePlanes() > 1)
		m_effect.SetTexture("g_UTexture", m_YUVPlanes[1]);
	if (YUVbuf->GetActivePlanes() > 2)
		m_effect.SetTexture("g_VTexture", m_YUVPlanes[2]);
	m_effect.SetFloatArray("g_StepXY", m_texSteps, ARRAYSIZE(m_texSteps));
}

bool YUV2RGBShader::UploadToGPU(YUVBuffer* YUVbuf)
{

	for (UINT i = 0; i < YUVbuf->GetActivePlanes(); i++) {
		ID3D10Resource* src, *dest;
		src = YUVbuf->planes[i].texture->GetResource();
		dest = m_YUVPlanes[i].GetResource();

		g_DXRendererPtr->GetDevice()->CopyResource(dest, src);
	}

	return true;
}

bool YUV2RGBShader::BuildVertexLayout()
{
	// Define the input layout
	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	D3D10_PASS_DESC pDesc;
	m_effect.GetTechnique()->GetPassByIndex(0)->GetDesc(&pDesc);

	ID3D10Device* pDevice = g_DXRendererPtr->GetDevice();
	HRESULT hr = pDevice->CreateInputLayout(layout, numElements, pDesc.pIAInputSignature, pDesc.IAInputSignatureSize, &m_inputLayout);
	if (SUCCEEDED(hr)) {
		return true;
	}

	LOGERR("Failed to create input Layout.");
	SAFE_RELEASE(m_inputLayout);
	return false;
}
