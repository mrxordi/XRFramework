#pragma once

class CTestMFCFrame;

// CChildWnd

class CChildView : public CWnd
{
	DECLARE_DYNAMIC(CChildView)

public:
	CChildView(CTestMFCFrame* pMainFrame);
	virtual ~CChildView();

	void SetVideoRect(const CRect& rect);	

protected:
	DECLARE_MESSAGE_MAP()
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);


	CWnd& GetWnd() {
		return *this;
	}

private:
	CRect m_vRect;
	CTestMFCFrame* m_mainFrame;

};