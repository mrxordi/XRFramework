#include "stdafxf.h"
#include "VideoBox.h"
#include "Toolbar.h"
#include "MainFrame.h"
#include "main.h"

VideoBox::VideoBox(wxWindow *parent, Context *ctx)
   : wxPanel(parent, -1), m_context(ctx), m_msInput(nullptr)
{
   wxToolBar* buttonToolbar = new Toolbar(this, "videonavigation", ctx, "", false);
   auto buttonSeek = new wxButton(this, IDB_Seek, "Seek To");
   auto buttonRead = new wxButton(this, IDB_Read, "Read Packet");
   m_msInput = new wxSpinCtrl(this, IDB_SecInput, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_LEFT, 0, 1000000, 0);



   auto videoBottomSizer = new wxBoxSizer(wxHORIZONTAL);
   videoBottomSizer->Add(buttonToolbar, wxSizerFlags(0).Center());
   videoBottomSizer->Add(buttonSeek, wxSizerFlags(0).Center());
   videoBottomSizer->Add(buttonRead, wxSizerFlags(0).Align(wxRight).Center ());
   videoBottomSizer->Add(m_msInput, wxSizerFlags(0).Align(wxRight).Center ());
   SetSizer(videoBottomSizer);
   
   buttonSeek->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &VideoBox::OnSeekButton, this);
   buttonRead->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &VideoBox::OnReadPacket, this);

}


VideoBox::~VideoBox()
{
}

void VideoBox::OnReadPacket(wxCommandEvent& event)
{
   uint8_t buf[2 * 1024];
   size_t out = wxGetApp().m_bufStream->Read(buf, sizeof(buf));
   LOGINFO("Readed %u from buffer. End %d.", out, wxGetApp().m_bufStream->GetCurrentReadPositon());
}

void VideoBox::OnSeekButton(wxCommandEvent& event)
{
   int val = m_msInput->GetValue();
   int64_t out = wxGetApp().m_bufStream->SeekTime(val);
   LOGINFO("Seek returned %d.", out);
}
