// FullscreenWnd.cpp : implementation file
//

#include "stdafx.h"
#include "FullscreenWnd.h"


// CFullscreenWnd

IMPLEMENT_DYNAMIC(CFullscreenWnd, CWnd)

CFullscreenWnd::CFullscreenWnd(CTestMFCFrame* mainFrame) : m_mainFrame(mainFrame)
{

}

CFullscreenWnd::~CFullscreenWnd()
{
}


BEGIN_MESSAGE_MAP(CFullscreenWnd, CWnd)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


BOOL CFullscreenWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs)) {
		return FALSE;
	}

	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		::LoadCursor(nullptr, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1), nullptr);

	return TRUE;
}

// CFullscreenWnd message handlers
BOOL CFullscreenWnd::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

bool CFullscreenWnd::IsWindow()
{
	return (m_hWnd != nullptr);
}
