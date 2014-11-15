#include "stdafxf.h"

#include "RenderSystemDX.h"
#include "render/ID3DResource.h"
//#include "window/DisplaySettings.h"

// singleton instance pointer
template<> cRenderSystemDX* Singleton<cRenderSystemDX>::ms_Singleton = 0;


cRenderSystemDX::cRenderSystemDX(void)
{
	m_bRenderCreated = false;
	m_bVSync = false;
	m_hDeviceWnd = NULL;
	m_hFocusWnd = NULL;
	m_bFullScreenDevice = false;
	m_refreshRate = 0.0f;
	m_inScene = false;
	m_desiredFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	m_pDevice = NULL;
	m_pSwapChain = NULL;
	m_pDepthStencilBuffer = NULL;
	m_pRenderTargetView = NULL;
	m_pDepthStencilView = NULL;
	m_pRasterizerState = NULL;

	m_pAdapter = NULL;
	m_pOutput = NULL;

	m_width = m_height = 0;
	m_renderControl = NULL;

}

cRenderSystemDX::~cRenderSystemDX(void)
{
}

bool cRenderSystemDX::InitRenderSystem(RenderControl* pControl)
{
	m_enumeration.Initialize();

	if (!XMVerifyCPUSupport()) {
		LOGERR("CPU doesn't support DirectXMath Library.");
		return false;
	}

	if (!pControl)
		return false;

	m_hFocusWnd = m_hDeviceWnd = pControl->GetHWND();

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));

	pControl->GetWH(m_width, m_height);
	m_renderControl = pControl;

	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.
	sd.BufferDesc.Width = m_width;
	sd.BufferDesc.Height = m_height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// No multisampling.
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	sd.Windowed = true;

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = m_hDeviceWnd;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	IDXGIAdapter* pAdapter = NULL;
	pAdapter = m_enumeration.GetAdapterInfo(0)->m_pAdapter;

	HR(D3D10CreateDeviceAndSwapChain(NULL,
		D3D10_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		D3D10_SDK_VERSION,
		&sd,
		&m_pSwapChain,
		&m_pDevice));

	if (!m_pDevice){
		LOGERR("(!)RenderSystem - D3D Device not created!");
		return false;
	}

	OnResize();

	//Distables using Alt+Enter event handling by Rendering system
	//and sending WM_SIZE message second time while window system handles it.
	//Still have to distable beep sound.
	IDXGIFactory *pFactory = NULL;
	HR(m_pSwapChain->GetParent(__uuidof(IDXGIFactory), (void **)&pFactory))
		pFactory->MakeWindowAssociation(m_hDeviceWnd, DXGI_MWA_NO_WINDOW_CHANGES);
	pFactory->Release();

	// tell any shared objects about our resurrection
	for (std::vector<ID3DResource *>::iterator i = m_resources.begin(); i != m_resources.end(); i++)
		(*i)->OnCreateDevice();

	m_bRenderCreated = true;

	return true;
}

bool cRenderSystemDX::DestroyRenderSystem()
{

	if (m_pSwapChain)
	{
		m_pSwapChain->SetFullscreenState(false, NULL);
	}
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pDepthStencilView);
	SAFE_RELEASE(m_pRasterizerState);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pDepthStencilBuffer);

	// tell any shared resources
	for (std::vector<ID3DResource *>::iterator i = m_resources.begin(); i != m_resources.end(); i++)
		(*i)->OnDestroyDevice();

	SAFE_RELEASE(m_pDevice);
	m_enumeration.Shutdown();

	LOGDEBUG("(-)RenderSystem unloaded.");
	return true;
}


void cRenderSystemDX::Register(ID3DResource *resource)
{
	CSingleLock lock(m_resourceSection);
	m_resources.push_back(resource);
}

void cRenderSystemDX::Unregister(ID3DResource *resource)
{
	CSingleLock lock(m_resourceSection);
	std::vector<ID3DResource*>::iterator i = find(m_resources.begin(), m_resources.end(), resource);
	if (i != m_resources.end())
		m_resources.erase(i);
}

bool cRenderSystemDX::OnResize()
{

	m_renderControl->GetWH(m_width, m_height);

	if (m_bRenderCreated) {
		for (std::vector<ID3DResource *>::iterator i = m_resources.begin(); i != m_resources.end(); i++)
			(*i)->OnFreeResources();
	}

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pDepthStencilView);
	SAFE_RELEASE(m_pDepthStencilBuffer);
	SAFE_RELEASE(m_pRasterizerState);

	//m_width = setwidth;
	//m_height = setheight;

	// Resize the swap chain and recreate the render target view.
	HR(m_pSwapChain->ResizeBuffers(1, m_width, m_height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D10Texture2D* backBufferPtr;
	HR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), reinterpret_cast<void**>(&backBufferPtr)));

	D3D10_TEXTURE2D_DESC backBufferDesc;
	backBufferPtr->GetDesc(&backBufferDesc);

	// Set the viewport transform.
	D3D10_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = backBufferDesc.Width;
	vp.Height = backBufferDesc.Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	m_pDevice->RSSetViewports(1, &vp);
	HR(m_pDevice->CreateRenderTargetView(backBufferPtr, 0, &m_pRenderTargetView));

	//release back buffer pointer as not needed
	SAFE_RELEASE(backBufferPtr);

	// Create depth stencil texture
	D3D10_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = backBufferDesc.Width;
	depthStencilDesc.Height = backBufferDesc.Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1; // multisampling must match
	depthStencilDesc.SampleDesc.Quality = 0; // swap chain values.
	depthStencilDesc.Usage = D3D10_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	HR(m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer));

	// Create the depth stencil view
	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = depthStencilDesc.Format;
	if (depthStencilDesc.SampleDesc.Count > 1)
		descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2DMS;
	else
		descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	HR(m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &descDSV, &m_pDepthStencilView));

	if (!m_pRasterizerState) {
		// Create a default rasterizer state that enables MSAA
		D3D10_RASTERIZER_DESC RSDesc;
		RSDesc.FillMode = D3D10_FILL_SOLID;
		RSDesc.CullMode = D3D10_CULL_BACK;
		RSDesc.FrontCounterClockwise = FALSE;
		RSDesc.DepthBias = 0;
		RSDesc.SlopeScaledDepthBias = 0.0f;
		RSDesc.DepthBiasClamp = 0;
		RSDesc.DepthClipEnable = TRUE;
		RSDesc.ScissorEnable = FALSE;
		RSDesc.AntialiasedLineEnable = FALSE;
		if (depthStencilDesc.SampleDesc.Count > 1)
			RSDesc.MultisampleEnable = TRUE;
		else
			RSDesc.MultisampleEnable = FALSE;

		HR(m_pDevice->CreateRasterizerState(&RSDesc, &m_pRasterizerState));
		m_pDevice->RSSetState(m_pRasterizerState);
	}

	if (!m_pDepthStencilView) {
		LOGERR("(!) GraphicLayer - No DepthStencilView");
		return false;
	}

	m_pDevice->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	LOGDEBUG("Renderer resized BackBuffer to: %i x %i .", backBufferDesc.Width, backBufferDesc.Height);

	if (m_bRenderCreated) {
		for (std::vector<ID3DResource *>::iterator i = m_resources.begin(); i != m_resources.end(); i++)
			(*i)->OnResizeDevice();
	}

	return true;
}

bool cRenderSystemDX::OnMove()
{
	return true;
}

bool cRenderSystemDX::BeginRender()
{
	if (!m_bRenderCreated)
		return false;

	float color[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	m_pDevice->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pDevice->ClearDepthStencilView(m_pDepthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0);

	// Restore default states, input layout and primitive topology 
	// because mFont->DrawText changes them.  Note that we can 
	// restore the default states by passing null.
	m_pDevice->OMSetDepthStencilState(0, 0);
	float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_pDevice->OMSetBlendState(0, blendFactors, 0xffffffff);

	m_inScene = true;

	return true;

}

bool cRenderSystemDX::EndRender()
{
	m_inScene = false;

	if (!m_bRenderCreated)
		return false;

	// Present the back buffer to the screen since rendering is complete.
	if (m_bVSync) {
		// Lock to screen refresh rate.
		m_pSwapChain->Present(1, 0);
	}
	else {
		// Present as fast as possible.
		m_pSwapChain->Present(0, 0);
	}

	return true;
}

bool cRenderSystemDX::PresentRender()
{

	return true;
}

void cRenderSystemDX::SetVSync(bool vsync)
{

}


