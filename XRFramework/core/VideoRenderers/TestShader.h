#pragma once
#include "CDX10Shader.h"
#include <DirectXMath.h>

using namespace DirectX;

class TestShader : public CDX10Shader
{
	struct CUSTOMVERTEX {
		XMFLOAT4 position;
		XMFLOAT4 color;
	};

public:
	TestShader(CDX10SystemRenderer *sys) : CDX10Shader(sys) {};
	virtual ~TestShader();

	virtual bool Create();
	virtual bool Render();
protected:
	virtual void PrepareParameters();
	virtual void SetShaderParameters();
	virtual bool BuildVertexLayout() override;
};

