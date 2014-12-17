#include "stdafxf.h"
#include "TestShader.h"
#include "render/RenderSystemDX.h"
#include "filesystem/File.h"

TestShader::~TestShader()
{

}

bool TestShader::Create()
{
	if (!CreateVertexBuffer(8, sizeof(CUSTOMVERTEX), 2)) {
		LOGFATAL("Failed to create Vertex Buffer. (No memory?)");
		return false;
	}

	if (!CreateIndexBuffer(5)) {
		LOGFATAL("Failed to create Vertex Buffer. (No memory?)");
		return false;
	}

	if (!LoadEffect("special://app/system/test.fx", nullptr)) {
		m_effect.Release();
		return false;
	}
	m_effect.SetTechnique("ColorTech");
	BuildVertexLayout();

	return true;
}

bool TestShader::Render()
{
	PrepareParameters();
	SetShaderParameters();
	Execute(NULL, 0);
	return true;
}

void TestShader::PrepareParameters()
{
	CUSTOMVERTEX* v;
	if (LockVertexBuffer((void**)&v)) {
		v[0].position = XMVectorSet(300.0f, 20.0f, 1.0f, 1.0f);
		v[0].color = XMVectorSet(0.5f, 0.5f, 1.0f, 1.0f);
		v[1].position = XMVectorSet(600.0f, 20.0f, 1.0f, 1.0f);
		v[1].color = XMVectorSet(0.8f, 0.2f, 1.0f, 1.0f);
		v[2].position = XMVectorSet(300.0f, 320.0f, 1.0f, 1.0f);
		v[2].color = XMVectorSet(0.1f, 0.9f, 1.0f, 1.0f);
		v[3].position = XMVectorSet(600.0f, 320.0f, 1.0f, 1.0f);
		v[3].color = XMVectorSet(0.5f, 0.8f, 4.0f, 1.0f);

		v[4].position = XMVectorSet(300.0f, 380.0f, 1.0f, 1.0f);
		v[4].color = XMVectorSet(0.5f, 0.5f, 1.0f, 1.0f);
		v[5].position = XMVectorSet(600.0f, 380.0f, 1.0f, 1.0f);
		v[5].color = XMVectorSet(0.8f, 0.2f, 1.0f, 1.0f);
		v[6].position = XMVectorSet(300.0f, 500.0f, 1.0f, 1.0f);
		v[6].color = XMVectorSet(0.1f, 0.9f, 1.0f, 1.0f);
		v[7].position = XMVectorSet(600.0f, 500.0f, 1.0f, 1.0f);
		v[7].color = XMVectorSet(0.5f, 0.8f, 4.0f, 1.0f);


		UnlockVertexBuffer();
	}

	int* f;
	if (LockIndexBuffer((void**)&f)) {
		f[0] = 3; 
		f[1] = 2;
		f[2] = 0;
		f[3] = 1;
		f[4] = 3;

		UnlockIndexBuffer();
	}
}

void TestShader::SetShaderParameters()
{
	m_effect.SetMatrix("gW", XMMATRIX((float*)g_DXRendererPtr->m_world.m));
	m_effect.SetMatrix("gV", XMMATRIX((float*)g_DXRendererPtr->m_view.m));
	m_effect.SetMatrix("gP", XMMATRIX((float*)g_DXRendererPtr->m_projection.m));
}

bool TestShader::BuildVertexLayout()
{
	// Define the input layout
	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D10_INPUT_PER_VERTEX_DATA, 0 },
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
