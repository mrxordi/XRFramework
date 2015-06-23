#pragma once
#include "wx/wx.h"
#include "Context.h"
class VideoBoxToolbar : public wxPanel
{
public:
   VideoBoxToolbar() {};
	VideoBoxToolbar(wxWindow *parent, Context *context);
	virtual ~VideoBoxToolbar();

   void OnReadPacket(wxCommandEvent& event);
   void OnSeekButton(wxCommandEvent& event);
   void OnLoadTestFrame(wxCommandEvent& event);
private:
   wxSpinCtrl* m_msInput;
   Context* m_context;
   DECLARE_DYNAMIC_CLASS_NO_COPY(VideoBoxToolbar);
};

