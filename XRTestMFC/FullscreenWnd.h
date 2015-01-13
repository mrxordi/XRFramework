#pragma once

class CTestMFCFrame;

// CFullscreenWnd

class CFullscreenWnd : public CWnd
{
	DECLARE_DYNAMIC(CFullscreenWnd)

public:
	CFullscreenWnd(CTestMFCFrame* mainFrame);
	virtual ~CFullscreenWnd();
	bool IsWindow();




	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
private:
	CTestMFCFrame* m_mainFrame;
};


