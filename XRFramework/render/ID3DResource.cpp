#include "stdafxf.h"
#include "ID3DResource.h"
#include "RenderSystemDX.h"
#include "Base.h"

const char* usages[] = { "D3D10_USAGE_DEFAULT", "D3D10_USAGE_IMMUTABLE", "D3D10_USAGE_DYNAMIC", "D3D10_USAGE_STAGING" };

/************************************************************************/
/*    D3DTexture                                                        */
/************************************************************************/
D3DTexture::D3DTexture(CDX10SystemRenderer *sys) : m_rendererSystem(sys)
{
	m_bLocked = false;
	m_resource = nullptr;
	m_shaderResourceView = nullptr;
	m_renderTargetView = nullptr;
}

D3DTexture::~D3DTexture()
{
	if ((m_shaderResourceView || m_renderTargetView) && m_resource)
	{
		Release();
	}
}

bool D3DTexture::Create(UINT width, UINT height, UINT bindflag, D3D10_USAGE usage, DXGI_FORMAT format, UINT mipLevels)
{
	if (!m_rendererSystem) {
		LOGFATAL("Failed to create texture, no system ptr provided");
	}
	m_mipLevels = mipLevels;
	m_format = format;
	m_bindflag = bindflag;
	m_width = width;
	m_height = height;
	ID3D10Device* pDevice = m_rendererSystem->GetDevice();

	DXGI_SAMPLE_DESC dxgiSampleDesc;
	dxgiSampleDesc.Count = 1;
	dxgiSampleDesc.Quality = 0;

	D3D10_TEXTURE2D_DESC d3d10Texture2DDesc;
	ZeroMemory(&d3d10Texture2DDesc, sizeof(d3d10Texture2DDesc));

	d3d10Texture2DDesc.Width = width;
	d3d10Texture2DDesc.Height = height;
	d3d10Texture2DDesc.MipLevels = mipLevels;
	d3d10Texture2DDesc.ArraySize = 1;
	d3d10Texture2DDesc.Format = format;
	d3d10Texture2DDesc.SampleDesc = dxgiSampleDesc;
	d3d10Texture2DDesc.Usage = usage;
	d3d10Texture2DDesc.BindFlags = bindflag;
	d3d10Texture2DDesc.CPUAccessFlags = (usage == D3D10_USAGE_DYNAMIC || usage == D3D10_USAGE_STAGING) ? D3D10_CPU_ACCESS_WRITE : 0;
	d3d10Texture2DDesc.MiscFlags = (mipLevels > 1) ? D3D10_RESOURCE_MISC_GENERATE_MIPS : 0;

	if (!pDevice) {
		LOGERR("Failed to create texture!");
		return false;
	}

	HRESULT hr = pDevice->CreateTexture2D(&d3d10Texture2DDesc, NULL, &m_resource);
	if (FAILED(hr))
	{
		LOGERR("Failed to create texture: 0x%08X", hr)
	}
	else {
		D3D10_TEXTURE2D_DESC desc;
		m_resource->GetDesc(&desc);

		if ((m_bindflag & D3D10_BIND_SHADER_RESOURCE) == D3D10_BIND_SHADER_RESOURCE) {

			if (desc.Format != m_format)
				LOGWARN("- format changed from %d to %d", m_format, desc.Format);
			if (desc.Height != m_height || desc.Width != m_width)
				LOGWARN("- size changed from %ux%u to %ux%u", m_width, m_height, desc.Width, desc.Height);

			D3D10_SHADER_RESOURCE_VIEW_DESC rtDesc;
			rtDesc.Format = d3d10Texture2DDesc.Format;
			rtDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
			rtDesc.Texture2D.MostDetailedMip = 0;
			rtDesc.Texture2D.MipLevels = mipLevels;
			HR(pDevice->CreateShaderResourceView(m_resource, &rtDesc, &m_shaderResourceView));

			if (!m_shaderResourceView) {
				LOGERR("Failed to create Shader Resource View!");
				return false;
			}
		}
		else if ((m_bindflag & D3D10_BIND_RENDER_TARGET) == D3D10_BIND_RENDER_TARGET) {
			D3D10_RENDER_TARGET_VIEW_DESC rtDesc;
			rtDesc.Format = desc.Format;
			rtDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
			rtDesc.Texture2D.MipSlice = 0;
			HR(pDevice->CreateRenderTargetView(m_resource, &rtDesc, &m_renderTargetView));

			if (!m_renderTargetView) {
				LOGERR("Failed to create Shader Render Target View!");
				return false;
			}
		}/* else {
			LOGERR("Unsupported Bind flags for texture resource!");
			Release();
			return false;
		}*/
		LOGINFO("Texture dimension %ux%u created with usage: %s", m_width, m_height, usages[usage]);

		m_rendererSystem->Register(this);
		return true;
	}
	return false;
}

bool D3DTexture::Lock(UINT level, D3D10_MAP typeofmap, D3D10_MAPPED_TEXTURE2D* mappedtexture)
{
	if (m_bLocked)
		return false;

	bool bResult = (m_resource->Map(D3D10CalcSubresource(0, 0, level), typeofmap, 0, mappedtexture) >= 0);
	if (bResult)
		m_bLocked = true;

	return m_bLocked;
}

bool D3DTexture::Unlock(UINT level)
{
	if (!m_bLocked)
		return false;

	m_resource->Unmap(D3D10CalcSubresource(0, 0, level));
	m_bLocked = false;
	return true;
}

void D3DTexture::Release()
{
	SAFE_RELEASE(m_shaderResourceView);
	SAFE_RELEASE(m_renderTargetView);
	SAFE_RELEASE(m_resource);
	m_rendererSystem->Unregister(this);
}

void D3DTexture::OnDestroyDevice()
{
	SAFE_RELEASE(m_shaderResourceView);
	SAFE_RELEASE(m_renderTargetView);
	SAFE_RELEASE(m_resource);
}

/************************************************************************/
/*    D3DEffect                                                         */
/************************************************************************/
D3DEffect::D3DEffect(CDX10SystemRenderer *sys) : m_rendererSystem(sys)
{
	m_effect = NULL;
	m_technique = NULL;
	m_defines.clear();
}

D3DEffect::~D3DEffect()
{
	if (m_effect)
		Release();
}

bool D3DEffect::Create(const std::string &effectString, DefinesMap* defines)
{
	if (!m_rendererSystem) {
		LOGFATAL("Failed to create effect. No render system provided.");
	}
	if (m_effect)
		return false;

	m_effectString = effectString;
	m_defines.clear();
	if (defines != NULL)
		m_defines = *defines; //FIXME: is this a copy of all members?
	if (CreateEffect())
	{
		m_rendererSystem->Register(this);
		return true;
	}

	SAFE_RELEASE(m_effect);
	m_defines.clear();
	return false;
}

void D3DEffect::Release()
{
	//SAFE_DELETE(m_technique);
	SAFE_RELEASE(m_effect);
	m_defines.clear();
	m_rendererSystem->Unregister(this);
}

bool D3DEffect::SetFloatArray(std::string handle, const float* val, unsigned int count)
{
	if (m_effect)
		return (m_effect->GetVariableByName(handle.c_str())->SetRawValue((void*)val, 0, count) >= 0);
	return false;
}

bool D3DEffect::SetMatrix(std::string handle, const XMFLOAT4X4& mat)
{
	if (m_effect)
		return (m_effect->GetVariableByName(handle.c_str())->AsMatrix()->SetMatrix((float*)mat.m) >= 0);

	return false;
}

bool D3DEffect::SetTechnique(std::string handle)
{
	if (m_effect)
		m_technique = m_effect->GetTechniqueByName(handle.c_str());
	return (m_technique != 0);
}

bool D3DEffect::SetTexture(std::string handle, D3DTexture &texture)
{
	if (m_effect)
		return (m_effect->GetVariableByName(handle.c_str())->AsShaderResource()->SetResource(texture.Get()) >= 0);
	return false;
}

bool D3DEffect::BeginPass(UINT pass)
{
	return (m_technique->GetPassByIndex(pass)->Apply(0) >= 0);
}

void D3DEffect::OnDestroyDevice()
{
	SAFE_RELEASE(m_effect);
	m_defines.clear();
}

bool D3DEffect::CreateEffect()
{
	if (m_effect)
		return false;
	std::vector<D3D10_SHADER_MACRO> definemacros;

	for (DefinesMap::const_iterator it = m_defines.begin(); it != m_defines.end(); ++it)
	{
		D3D10_SHADER_MACRO m;
		m.Name = it->first.c_str();
		if (it->second.empty())
			m.Definition = NULL;
		else
			m.Definition = it->second.c_str();
		definemacros.push_back(m);
	}

	definemacros.push_back(D3D10_SHADER_MACRO());
	definemacros.back().Name = 0;
	definemacros.back().Definition = 0;

	// create the main effect from the shader source.
	ID3D10Blob* errors = 0;
	//if (FAILED(D3DX10CreateEffectFromMemory(shaderSource, sizeof(shaderSource),
	//	0, 0, 0, "fx_4_0", 0, 0, d_device,
	//	0, 0, &d_effect, &errors, 0)))
	//{
	//	std::string msg(static_cast<const char*>(errors->GetBufferPointer()),
	//		errors->GetBufferSize());
	//	errors->Release();
	//	CEGUI_THROW(RendererException(msg));
	//}
	ID3D10Blob* compiledeffect = 0;
	UINT HSLSflags = 0;
	HSLSflags |= D3D10_SHADER_ENABLE_STRICTNESS;
#ifdef _DEBUG
	HSLSflags |= D3D10_SHADER_DEBUG;
#endif
	if (FAILED(D3D10CompileEffectFromMemory((void*)m_effectString.c_str(), m_effectString.length(), 0, &definemacros[0], 0, D3D10_SHADER_ENABLE_STRICTNESS, 0, &compiledeffect, &errors)))
	{
		std::string msg(static_cast<const char*>(errors->GetBufferPointer()),
			errors->GetBufferSize());
		LOGERR("%s", msg.c_str());
		errors->Release();
		compiledeffect->Release();
		return false;
	}

	if (FAILED(D3D10CreateEffectFromMemory(compiledeffect->GetBufferPointer(), compiledeffect->GetBufferSize(), 0, m_rendererSystem->GetDevice(), 0, &m_effect)))
	{
		std::string msg(static_cast<const char*>(errors->GetBufferPointer()),
			errors->GetBufferSize());
		LOGERR("%s", msg.c_str());
		errors->Release();
		compiledeffect->Release();
		return false;
	}
	compiledeffect->Release();

	return true;
}



/************************************************************************/
/*    D3DVertexBuffer                                                    */
/************************************************************************/
D3DVertexBuffer::D3DVertexBuffer(CDX10SystemRenderer *sys) : m_rendererSystem(sys)
{
	m_bIsValid = false;
	m_length = 0;
	m_indexBuffer = NULL;
	m_type = NONINDEXED_BUFFER;
	m_indexBuffer = NULL;
	m_vertexBuffer = NULL;
}

D3DVertexBuffer::~D3DVertexBuffer()

{
	if (m_bIsValid)
		Release();
}

bool D3DVertexBuffer::CreateBuffer(UINT length, D3D10_USAGE usage /*= D3D10_USAGE_DEFAULT*/, void* data /*= NULL*/)
{
	if (m_vertexBuffer)
		return false;

	if (usage == D3D10_USAGE_IMMUTABLE && data == NULL) {
		LOGERR("No data provided while creating %s buffer", usages[usage]);
		return false;
	}
	D3D10_BUFFER_DESC vertexBufferDesc;

	// Set up the description of the vertex buffer.
	vertexBufferDesc.Usage = usage;
	vertexBufferDesc.ByteWidth = length;
	vertexBufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = (usage == D3D10_USAGE_DYNAMIC) ? D3D10_CPU_ACCESS_WRITE : 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA InitData;
	if (data) {
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = data;
	}

	HRESULT hhr = -1L;

	if (m_rendererSystem->GetDevice()) {
		hhr = m_rendererSystem->GetDevice()->CreateBuffer(&vertexBufferDesc, data ? &InitData : NULL, &m_vertexBuffer);
	}
	if (SUCCEEDED(hhr)) {
		m_length = length;
		m_bIsValid = true;
		m_rendererSystem->Register(this);
		return true;
	}
	else
		HR(hhr);

	SAFE_RELEASE(m_vertexBuffer);
	return false;
}

bool D3DVertexBuffer::CreateIndex(UINT length, D3D10_USAGE usage /*= D3D10_USAGE_DEFAULT*/, void* data /*= NULL*/)
{
	if (m_indexBuffer)
		return false;

	if (usage == D3D10_USAGE_IMMUTABLE && data == NULL) {
		LOGERR("No data provided while creating %s index buffer", usages[usage]);
		return false;
	}
	D3D10_BUFFER_DESC indexBufferDesc;

	// Set up the description of the vertex buffer.
	indexBufferDesc.Usage = usage;
	indexBufferDesc.ByteWidth = length;
	indexBufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = (usage == D3D10_USAGE_DYNAMIC) ? D3D10_CPU_ACCESS_WRITE : 0;
	indexBufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA InitData;
	if (data) {
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = data;
	}

	HRESULT hhr = -1;
	if (m_rendererSystem->GetDevice()) {
		hhr = m_rendererSystem->GetDevice()->CreateBuffer(&indexBufferDesc, data ? &InitData : NULL, &m_indexBuffer);
	}
	if (SUCCEEDED(hhr)) {
		m_type = INDEXED_BUFFER;
		return true;
	}
	else
		HR(hhr);

	SAFE_RELEASE(m_indexBuffer);
	m_type = NONINDEXED_BUFFER;
	return false;
}

void D3DVertexBuffer::Release()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
	m_type = NONINDEXED_BUFFER;
	m_bIsValid = false;
	m_rendererSystem->Unregister(this);
}

bool D3DVertexBuffer::LockBuffer(D3D10_MAP mapType, void **data)
{
	return (m_vertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, NULL, data) >= 0);
}

void D3DVertexBuffer::UnlockBuffer()
{
	m_vertexBuffer->Unmap();
}

bool D3DVertexBuffer::LockIndex(D3D10_MAP mapType, void **data)
{
	return (m_indexBuffer->Map(D3D10_MAP_WRITE_DISCARD, NULL, data) >= 0);
}

void D3DVertexBuffer::UnlockIndex()
{
	m_indexBuffer->Unmap();
}

void D3DVertexBuffer::OnDestroyDevice()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
	m_type = NONINDEXED_BUFFER;
	m_bIsValid = false;
}
