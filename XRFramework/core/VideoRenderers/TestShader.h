#pragma once
#include "WinShader.h"
#include <DirectXMath.h>

using namespace DirectX;

class TestShader : public WinShader
{
	struct CUSTOMVERTEX {
		XMFLOAT4 position;
		XMFLOAT4 color;
	};

public:
	TestShader() {};
	virtual ~TestShader();

	virtual bool Create();
	virtual bool Render();
protected:
	virtual void PrepareParameters();
	virtual void SetShaderParameters();
	virtual bool BuildVertexLayout() override;
};

