#include "stdafxf.h"
#include "WinShader.h"
#include "filesystem/File.h"
#include "filesystem/StreamFile.h"
#include "render/RenderSystemDX.h"

WinShader::~WinShader()
{
	if (m_effect.Get())
		m_effect.Release();
	if (m_vb.GetVertexBuffer() || m_vb.GetIndexBuffer())
		m_vb.Release();

	SAFE_RELEASE(m_inputLayout);
}

bool WinShader::CreateVertexBuffer(unsigned int vertCount, unsigned int vertSize, unsigned int primitivesCount)
{
	if (!m_vb.CreateBuffer(vertCount*vertSize, D3D10_USAGE_DYNAMIC))
		return false;

	m_vbsize = vertCount*vertSize;
	m_vertsize = vertSize;
	m_primitivesCount = primitivesCount;
	return true;
}

bool WinShader::CreateIndexBuffer(unsigned int indexCount)
{
	if (!m_vb.CreateIndex(indexCount*sizeof(int), D3D10_USAGE_DYNAMIC))
		return false;

	m_indexCount = indexCount;
	return true;
}

bool WinShader::LockVertexBuffer(void **data)
{
	if (m_vb.LockBuffer(D3D10_MAP_WRITE_DISCARD, data))
		return true;

	LOGERR("Failed to map vertex buffer.");
	return false;
}

bool WinShader::LockIndexBuffer(void **data)
{
	if (m_vb.LockIndex(D3D10_MAP_WRITE_DISCARD, data))
		return true;

	LOGERR("Failed to map index buffer.");
	return false;
}

void WinShader::UnlockVertexBuffer()
{
	m_vb.UnlockBuffer();
}

void WinShader::UnlockIndexBuffer()
{
	m_vb.UnlockIndex();
}

bool WinShader::LoadEffect(const std::string& filename, DefinesMap* defines)
{
	LOGDEBUG(" loading shader %s", filename.c_str());

	CFileStream file;

	if (!file.Open(filename)) 
	{
		LOGERR(" failed to open file %s", filename.c_str());
		return false;
	}

	//Get whole file to string
	std::string pStrEffect;
	getline(file, pStrEffect, '\0');

	if (!m_effect.Create(pStrEffect, defines))
	{
		LOGERR(" failed to create effect", pStrEffect.c_str());
		return false;
	}
	return true;
}

bool WinShader::Execute(std::vector<ID3D10RenderTargetView*> *vecRT, unsigned int vertexIndexStep)
{
	ID3D10Device* pDevice = g_DXRendererPtr->GetDevice();
	ID3D10RenderTargetView* oldRTV = 0;
	ID3D10DepthStencilView* oldDSV = 0;

	if (vecRT!=NULL && !vecRT->empty())
		pDevice->OMGetRenderTargets(1, &oldRTV, &oldDSV);

	D3D10_TECHNIQUE_DESC techDesc;
	m_effect.GetTechnique()->GetDesc(&techDesc);
	unsigned int cPasses = techDesc.Passes;

	unsigned int stride = m_vertsize;
	unsigned int offset = 0;
	ID3D10Buffer* buffer = m_vb.GetVertexBuffer();
	pDevice->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	if (m_vb.m_type == D3DVertexBuffer::INDEXED_BUFFER) 
	{
		pDevice->IASetIndexBuffer(m_vb.GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	} else 
	{
		pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	}

	pDevice->IASetInputLayout(m_inputLayout);
	for (unsigned int iPass = 0; iPass < cPasses; iPass++)
	{
		if (!m_effect.BeginPass(iPass)) 
		{
			LOGERR("Failed to apply and begin pass %u in technique %s", iPass, techDesc.Name);
			break;
		}

		if (vecRT != NULL && vecRT->size() > iPass)
			pDevice->OMSetRenderTargets(1, &(*vecRT)[iPass], NULL);

		if (m_vb.m_type == D3DVertexBuffer::INDEXED_BUFFER && m_primitivesCount > 0) 
			for (int i = 0; i < m_primitivesCount; i++) 
				pDevice->DrawIndexed(m_indexCount, 0, iPass*vertexIndexStep+i*((m_vbsize/m_vertsize)/m_primitivesCount));
		else
			pDevice->Draw(m_primitivesCount, iPass*vertexIndexStep);
	}

	if (oldRTV != 0) 
	{
		if (oldDSV != 0) 
		{
			pDevice->OMSetRenderTargets(1, &oldRTV, oldDSV);
			oldRTV->Release();
			oldRTV->Release();
		} else 
		{
			pDevice->OMSetRenderTargets(1, &oldRTV, NULL);
			oldRTV->Release();
		}
	}
	pDevice->RSSetViewports(1, &g_DXRendererPtr->GetViewPort());
	return true;
}