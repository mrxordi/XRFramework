#pragma once
#include "wx/wx.h"
#include "Context.h"
class VideoBox : public wxPanel
{
public:
	VideoBox(wxWindow *parent, Context *context);
	virtual ~VideoBox();

   void OnReadPacket(wxCommandEvent& event);
   void OnSeekButton(wxCommandEvent& event);
private:
   wxSpinCtrl* m_msInput;
   Context* m_context;
};

