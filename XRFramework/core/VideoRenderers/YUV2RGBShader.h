#pragma once
#include "WinShader.h"
#include <d3d10.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <memory>
#include "RenderModes.h"
#include "YUV2RGBMatrix.h"
#include "render/ID3DResource.h"
#include "utils/XRect.h"

using namespace DirectX;

#define MAX_PLANES 3
#define PLANE_Y 0
#define PLANE_U 1
#define PLANE_V 2
#define PLANE_UV 1

//###############################################
//#  SVideoPlane
//###############################################
struct SVideoPlane
{
	std::unique_ptr<D3DTexture> texture;
	D3D10_MAPPED_TEXTURE2D rect;                  // rect.pBits != NULL is used to know if the texture is locked
};

//###############################################
//#  YUVBuffer
//###############################################
class YUVBuffer
{
public:
	YUVBuffer();
	virtual ~YUVBuffer();
	
	bool Create(ERenderFormat format, unsigned int width, unsigned int height);
	virtual void Release();
	virtual void StartDecode();
	virtual void StartRender();
	virtual void Clear();
	unsigned int GetActivePlanes() { return m_activeplanes; }
	virtual bool IsReadyToRender();

	SVideoPlane planes[MAX_PLANES];
private:
	unsigned int     m_width;
	unsigned int     m_height;
	ERenderFormat    m_format;
	unsigned int     m_activeplanes;
	bool             m_locked;
};


//###############################################
//#  YUV2RGBShader
//###############################################
class YUV2RGBShader : public WinShader
{
	struct CUSTOMVERTEX {
		XMFLOAT4 coord;
		XMFLOAT2 tyv;	 // Y Texture coordinates
		XMFLOAT2 tuv;	 // U Texture coordinates
		XMFLOAT2 tvv;	 // V Texture coordinates
	};
public:
	YUV2RGBShader() :
		m_sourceWidth(0),
		m_sourceHeight(0),
		m_format(RENDER_FMT_NONE)
	{
		memset(&m_texSteps, 0, sizeof(m_texSteps));
	}
	virtual ~YUV2RGBShader();

	bool Create(UINT srcWidth, UINT srcHeight, ERenderFormat format);
	virtual void Render(XRect sourceRect, XRect destRect, float contrast, float brightness,
		unsigned int flags,
		YUVBuffer* YUVbuf);

protected:
	virtual void PrepareParameters(XRect sourceRect, XRect destRect, float contrast, float brightness, unsigned int flags);
	virtual void SetShaderParameters(YUVBuffer* YUVbuf);
	virtual bool UploadToGPU(YUVBuffer* YUVbuf);

	virtual bool BuildVertexLayout() override;

private:
	YUV2RGBMatrix m_matrix;
	unsigned int   m_sourceWidth, m_sourceHeight;
	XRect          m_sourceRect, m_destRect;
	ERenderFormat  m_format;
	D3DTexture    m_YUVPlanes[3];
	float          m_texSteps[2];
};

