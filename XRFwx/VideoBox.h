#pragma once
#include "wx/wx.h"
#include "Context.h"
class VideoBox : public wxPanel
{
public:
	VideoBox(wxWindow *parent, Context *context);
	virtual ~VideoBox();
private:
   Context* m_context;
};

