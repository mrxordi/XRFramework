#pragma once
#include "TestMFCApp.h"
#include "ChildView.h"
#include "FullscreenWnd.h"
#include "../XRFramework/render/RenderSystemDX.h"
#include "../XRFramework/core/VideoRenderers/YUV2RGBShader.h"

#define MPC_WND_CLASS_NAME_FULSCREEN L"TestMFCApplication Fullscreen Window!"

class CTestMFCFrame : public CFrameWnd
{
public:
	DECLARE_DYNAMIC(CTestMFCFrame)

	CTestMFCFrame() ;
	virtual ~CTestMFCFrame();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	bool CreateFullScreenWindow();
	bool IsD3DFullScreenMode();
	void SetDefaultWindowSize();

	void SaveSettings();


	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnRender();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnterSizeMove();
	afx_msg void OnExitSizeMove();
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
protected:
	CChildView m_wndView;
	CFullscreenWnd* m_fullScreenWnd;

private:
	bool m_bFullScreen;
	bool m_isMinimized,
		m_isMaximized,
		m_isResizing,
		m_isMoving,
		m_bRender;
	CRect m_currentRect;

	YUV2RGBShader m_yuvshader;
	YUVBuffer m_yuvbuffer;
public:
	afx_msg void OnMove(int x, int y);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
};

