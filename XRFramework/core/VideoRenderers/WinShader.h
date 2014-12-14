#pragma once
#include <vector>
#include "render/ID3DResource.h"

class WinShader
{
protected:
	WinShader() :
		m_vbsize(0),
		m_vertsize(0),
		m_primitivesCount(0),
		m_indexCount(0)
	{};
	virtual ~WinShader();
	virtual bool CreateVertexBuffer(unsigned int vertCount, unsigned int vertSize, unsigned int primitivesCount);
	virtual bool CreateIndexBuffer(unsigned int indexCount);

	virtual bool LockVertexBuffer(void **data);
	virtual bool LockIndexBuffer(void **data);

	virtual void UnlockVertexBuffer();
	virtual void UnlockIndexBuffer();

	virtual bool LoadEffect(const std::string& filename, DefinesMap* defines);
	virtual bool BuildVertexLayout() = 0;
	virtual bool Execute(std::vector<ID3D10RenderTargetView*> *vecRT, unsigned int vertexIndexStep);


	D3DEffect m_effect;
	ID3D10InputLayout* m_inputLayout;

private:
	D3DVertexBuffer m_vb;
	unsigned int     m_vbsize;
	unsigned int     m_vertsize;
	unsigned int     m_primitivesCount;
	unsigned int	 m_indexCount;
};