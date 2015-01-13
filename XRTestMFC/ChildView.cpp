// ChildWnd.cpp : implementation file
//

#include "stdafx.h"
#include "ChildView.h"
#include "../XRCommon/utils/Singleton.h"
#include "../XRFramework/render/RenderSystemDX.h"


// CChildWnd

IMPLEMENT_DYNAMIC(CChildView, CWnd)

CChildView::CChildView(CTestMFCFrame* pMainFrame) : m_mainFrame(pMainFrame), m_vRect(0, 0, 0, 0)
{
}

CChildView::~CChildView()
{
}


void CChildView::SetVideoRect(const CRect& rect)
{
	m_vRect = rect;
	Invalidate();
}

BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_CREATE()
END_MESSAGE_MAP()



// CChildWnd message handlers
void CChildView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}


LRESULT CChildView::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	return CWnd::OnNcHitTest(point);
}


void CChildView::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnNcLButtonDown(nHitTest, point);
}


BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs)) {
		return FALSE;
	}

	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		::LoadCursor(nullptr, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1), nullptr);

	return TRUE;
}


BOOL CChildView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	return CWnd::PreTranslateMessage(pMsg);
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}
