#pragma once
#include <vector>
#include <d3d10.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Base.h"
#include "utils/Singleton.h"
#include "render/D3D10Enumeration.h"
#include "XRThreads/CriticalSection.h"
#include "XRThreads/SingleLock.h"
#include "log/Log.h"
//#include "window/Resolution.h"
#include "window/RenderControl.h"


using namespace DirectX;
using namespace PackedVector;

class ID3DResource;

class cRenderSystemDX : public Singleton < cRenderSystemDX >
{
public:
	cRenderSystemDX(void);
	virtual ~cRenderSystemDX(void);

	static cRenderSystemDX& Create() { return *new cRenderSystemDX(); }
	static void				Destroy() { delete cRenderSystemDX::getSingletonPtr(); }

	virtual bool InitRenderSystem(RenderControl* pControl = NULL, HWND hwnd = NULL);
	virtual bool DestroyRenderSystem();
	bool OnResize();
	virtual bool OnMove();
	virtual bool BeginRender();
	virtual bool EndRender();
	virtual bool PresentRender();

	/*!
	\brief Register as a dependent of the DirectX Render System
	Resources should call this on construction if they're dependent on the Render System
	for survival. Any resources that registers will get callbacks on loss and reset of
	device, where resources that are in the D3DPOOL_DEFAULT pool should be handled.
	In addition, callbacks for destruction and creation of the device are also called,
	where any resources dependent on the DirectX device should be destroyed and recreated.
	\sa Unregister, ID3DResource
	*/
	void Register(ID3DResource *resource);

	/*!
	\brief Unregister as a dependent of the DirectX Render System
	Resources should call this on destruction if they're a dependent on the Render System
	\sa Register, ID3DResource
	*/
	void Unregister(ID3DResource *resource);



	ID3D10Device* GetDevice() { return m_pDevice; }
	bool RendererCreated() { return m_bRenderCreated; }
	void SetFocusWnd(HWND wnd) { m_hFocusWnd = wnd; }
	//void SetDeviceWnd(HWND wnd) { m_hDeviceWnd = wnd; }
	virtual void SetVSync(bool vsync);
	bool GetVSync() { return m_bVSync; }
	int GetWidth() { return m_width; }
	int GetHeight() { return m_height; }
	void SetCameraPosition(XMFLOAT2 &camera, int screenWidth, int screenHeight);
	D3D10_VIEWPORT GetViewPort() { return m_viewPort; }

	XMFLOAT4X4 m_projection;
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_world;

	struct {
		XMFLOAT4X4 m_projection;
		XMFLOAT4X4 m_view;
		XMFLOAT4X4 m_world;
	} m_matMVP;
protected:


protected:
	bool            m_bRenderCreated;
	bool			m_bVSync;
	HWND			m_hFocusWnd;
	HWND			m_hDeviceWnd;
	bool			m_bFullScreenDevice;
	float			m_refreshRate;
	bool			m_inScene;
	int				m_width;
	int				m_height;
	DXGI_FORMAT		m_desiredFormat;
	DXGI_MODE_DESC  m_modeDesc;
	DXGI_SWAP_CHAIN_DESC m_SwapChainDesc;

	D3D10Enumeration m_enumeration;

	XR::CCriticalSection            m_resourceSection;
	std::vector<ID3DResource*>  m_resources;

	ID3D10Device*		  m_pDevice;
	IDXGISwapChain*		  m_pSwapChain;
	ID3D10Texture2D*	  m_pDepthStencilBuffer;
	ID3D10RenderTargetView* m_pRenderTargetView;
	ID3D10DepthStencilView* m_pDepthStencilView;
	ID3D10RasterizerState*  m_pRasterizerState;

	IDXGIAdapter* m_pAdapter;  //Current used adapter
	IDXGIOutput*  m_pOutput;   //Current used Output
	RenderControl* m_renderControl;

	std::string		m_RenderRenderer;
	std::string		m_RenderVendor;
	std::string		m_RenderVersion;

	D3D10_VIEWPORT m_viewPort;
};


#define g_DXRenderer cRenderSystemDX::getSingleton()
#define g_DXRendererPtr cRenderSystemDX::getSingletonPtr()