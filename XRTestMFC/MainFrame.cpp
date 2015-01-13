#include "stdafx.h"
#include "MainFrame.h"
#include "../XRCommon/log/Log.h"
#include "../XRFramework/core/VideoRenderers/WinRenderer.h"
#include "../XRFramework/filesystem/File.h"

IMPLEMENT_DYNAMIC(CTestMFCFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CTestMFCFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()

	ON_WM_SIZE()
	ON_WM_ENTERSIZEMOVE()
	ON_WM_EXITSIZEMOVE()
	ON_WM_SIZING()
	ON_WM_MOVE()
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()


CTestMFCFrame::CTestMFCFrame() : m_bFullScreen(false), m_wndView(this), m_bRender(true), m_isResizing(false), m_isMoving(false), m_isMinimized(false), m_isMaximized(false), m_currentRect(0, 0, 0, 0)
{
}

CTestMFCFrame::~CTestMFCFrame()
{
	cRenderSystemDX::Destroy();
}

int CTestMFCFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	if (!m_wndView.Create(nullptr, nullptr, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, nullptr)) {
		LOGFATAL("Failed to create view for main frame");
	}

	cRenderSystemDX::Create();
	if (!g_DXRendererPtr->InitRenderSystem(NULL, GetSafeHwnd())) {
		LOGFATAL("Failed to init render system");
	}

	// Should never be RTLed
	m_wndView.ModifyStyleEx(WS_EX_LAYOUTRTL, WS_EX_NOINHERITLAYOUT);

	m_fullScreenWnd = DEBUG_NEW CFullscreenWnd(this);

	SetFocus();

	m_yuvshader.Create(1280, 534, RENDER_FMT_YUV420P);
	m_yuvbuffer.Create(RENDER_FMT_YUV420P, 1280, 534);
	m_yuvbuffer.StartDecode();
	UINT dstPitch[3], dstPitchBefore[3];
	unsigned char* dstData[3], *dstDataBefore[3];

	for (UINT i = 0; i < 3; i++) {
		dstPitchBefore[i] = dstPitch[i] = m_yuvbuffer.planes[i].rect.RowPitch;
		dstDataBefore[i] = dstData[i] = (unsigned char*)m_yuvbuffer.planes[i].rect.pData;
	}

	File fe;
	auto_buffer buff;
	const char* filelist[3] = { "special://app/system/frame.yyy", "special://app/system/frame.uuu", "special://app/system/frame.vvv" };
	YV12Image image;
	{
		fe.LoadFile(filelist[0], buff);
		uint8_t* dataY = new uint8_t[buff.size()];
		memcpy(dataY, buff.get(), buff.size());
		image.plane[0] = dataY;
		image.planesize[0] = buff.size();
		image.stride[0] = 1280;

		fe.LoadFile(filelist[1], buff);
		uint8_t* dataU = new uint8_t[buff.size()];
		memcpy(dataU, buff.get(), buff.size());
		image.plane[1] = dataU;
		image.planesize[1] = buff.size();
		image.stride[1] = 640;


		fe.LoadFile(filelist[2], buff);
		uint8_t* dataV = new uint8_t[buff.size()];
		memcpy(dataV, buff.get(), buff.size());
		image.plane[2] = dataV;
		image.planesize[2] = buff.size();
		image.stride[2] = 640;

		image.width = 1280;
		image.height = 534;
		image.cshift_x = 1;
		image.cshift_y = 1;
	}

	uint8_t* s = image.plane[0];
	uint8_t* d = (BYTE*)m_yuvbuffer.planes[0].rect.pData;
	int w = m_yuvbuffer.planes[0].texture.GetWidth();
	int h = m_yuvbuffer.planes[0].texture.GetHeight();

	for (int i = 0; i < h; i++) {
		memcpy(d, s, w);
		s += image.stride[0];
		d += m_yuvbuffer.planes[0].rect.RowPitch;
	}

	s = image.plane[1];
	d = (BYTE*)m_yuvbuffer.planes[1].rect.pData;
	w = (m_yuvbuffer.planes[1].texture.GetWidth());
	h = (m_yuvbuffer.planes[1].texture.GetHeight());

	for (int i = 0; i < h; i++) {
		memcpy(d, s, w);
		s += image.stride[1];
		d += m_yuvbuffer.planes[1].rect.RowPitch;
	}

	s = image.plane[2];
	d = (BYTE*)m_yuvbuffer.planes[2].rect.pData;

	for (int i = 0; i < h; i++) {
		memcpy(d, s, w);
		s += image.stride[2];
		d += m_yuvbuffer.planes[2].rect.RowPitch;
	}


	m_yuvbuffer.StartRender();

	delete image.plane[0];
	delete image.plane[1];
	delete image.plane[2];


	LOGINFO("---> Main frame created");
	return S_OK;
}

void CTestMFCFrame::OnDestroy() {

	g_DXRendererPtr->DestroyRenderSystem();

	if (m_fullScreenWnd) {
		if (m_fullScreenWnd->IsWindow()) {
			m_fullScreenWnd->DestroyWindow();
		}
		delete m_fullScreenWnd;
	}

	__super::OnDestroy();
	LOGINFO("---> Main frame destroing");
}

BOOL CTestMFCFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!__super::PreCreateWindow(cs)) {
		return FALSE;
	}

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = MPC_WND_CLASS_NAME;
	return TRUE;
}


bool CTestMFCFrame::CreateFullScreenWindow()
{
	HMONITOR      hMonitor;
	MONITORINFOEX MonitorInfo;
	CRect         MonitorRect;

	if (m_fullScreenWnd->IsWindow()) {
		m_fullScreenWnd->DestroyWindow();
	}
	return true;
}

bool CTestMFCFrame::IsD3DFullScreenMode()
{
	return m_fullScreenWnd && m_fullScreenWnd->IsWindow();
}


void CTestMFCFrame::OnRender()
{
	if (m_bRender)
		if (g_DXRendererPtr->BeginRender()) {
			m_yuvshader.Render(XRect(0, 0, 1280, 534), XRect(5, 5, m_currentRect.Width()-5, m_currentRect.Height()-5), 50.0f, 50.0f, CONF_FLAGS_YUVCOEF_BT601, &m_yuvbuffer);
		g_DXRendererPtr->EndRender();
		}	
}

void CTestMFCFrame::OnSize(UINT nType, int cx, int cy)
{
	if (!m_isResizing) {
		switch (nType)
		{
		case SIZE_MAXIMIZED:
			if (m_isMinimized)
				m_isMinimized = false;

			m_isMaximized = true;
			break;
		case SIZE_MINIMIZED:
			m_isMinimized = true;
			break;
		case SIZE_RESTORED:
			if (m_isMaximized && !m_isMinimized)
				m_isMaximized = false;

			if (m_isMinimized)
				m_isMinimized = false;
			break;
		default:
			break;
		}

		if (m_isMinimized && m_bRender)
			m_bRender = false;
		else if (!m_isMinimized && !m_bRender)
			m_bRender = true;
	}

	__super::OnSize(nType, cx, cy);

	if ((m_currentRect.Height() != cy || m_currentRect.Width() != cx) && !m_isMinimized && !m_isResizing)
	{
			g_DXRendererPtr->OnResize();
			GetClientRect(&m_currentRect);
	}
}

void CTestMFCFrame::OnEnterSizeMove()
{
	m_isMoving = m_isResizing = true;

	//do not render while resizing window 
	m_bRender = false;
	GetClientRect(m_currentRect);

	__super::OnEnterSizeMove();
}

void CTestMFCFrame::OnExitSizeMove()
{
	m_isMoving = m_isResizing = false;
	CRect newRect;
	GetClientRect(&newRect);
	if ((newRect.Width() != m_currentRect.Width() || newRect.Height() != m_currentRect.Height()) && !m_isMinimized)
	{
		g_DXRendererPtr->OnResize();
		m_currentRect = newRect;
	}
	m_bRender = true;

	__super::OnExitSizeMove();
}

void CTestMFCFrame::OnSizing(UINT fwSide, LPRECT pRect)
{
	CFrameWnd::OnSizing(fwSide, pRect);

	// TODO: Add your message handler code here
}

void CTestMFCFrame::OnMove(int x, int y)
{
	CFrameWnd::OnMove(x, y);

	// TODO: Add your message handler code here
}

void CTestMFCFrame::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	if (m_bFullScreen && !(lpwndpos->flags & SWP_NOMOVE)) {
		HMONITOR hm = MonitorFromPoint(CPoint(lpwndpos->x, lpwndpos->y), MONITOR_DEFAULTTONULL);
		MONITORINFO mi = { sizeof(mi) };
		if (GetMonitorInfo(hm, &mi)) {
			lpwndpos->flags &= ~SWP_NOSIZE;
			lpwndpos->cx = mi.rcMonitor.right - mi.rcMonitor.left;
			lpwndpos->cy = mi.rcMonitor.bottom - mi.rcMonitor.top;
			lpwndpos->x = mi.rcMonitor.left;
			lpwndpos->y = mi.rcMonitor.top;
		}
	}
	__super::OnWindowPosChanging(lpwndpos);
}
