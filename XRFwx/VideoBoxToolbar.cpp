#include "stdafxf.h"
#include "VideoBoxToolbar.h"
#include "Toolbar.h"
#include "MainFrame.h"
#include "main.h"

IMPLEMENT_DYNAMIC_CLASS(VideoBoxToolbar, wxPanel)

VideoBoxToolbar::VideoBoxToolbar(wxWindow *parent, Context *ctx)
   : wxPanel(parent, -1), m_context(ctx), m_msInput(nullptr)
{
   wxToolBar* buttonToolbar = new Toolbar(this, "videonavigation", ctx, "", false);
   auto buttonSeek = new wxButton(this, IDB_Seek, "Seek To");
   auto buttonLoadTestFrame = new wxButton(this, IDB_LoadTestFrame, "Load test Frame");
   auto buttonRead = new wxButton(this, IDB_Read, "Read Packet");
   m_msInput = new wxSpinCtrl(this, IDB_SecInput, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_LEFT, 0, 1000000, 0);



   auto videoBottomSizer = new wxBoxSizer(wxHORIZONTAL);
   videoBottomSizer->Add(buttonToolbar, wxSizerFlags(0).Center());
   videoBottomSizer->Add(buttonSeek, wxSizerFlags(0).Center());
   videoBottomSizer->Add(buttonLoadTestFrame, wxSizerFlags(0).Center());
   videoBottomSizer->Add(buttonRead, wxSizerFlags(0).Align(wxRight).Center ());
   videoBottomSizer->Add(m_msInput, wxSizerFlags(0).Align(wxRight).Center ());
   SetSizer(videoBottomSizer);
   
   buttonSeek->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &VideoBoxToolbar::OnSeekButton, this);
   buttonRead->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &VideoBoxToolbar::OnReadPacket, this);
   buttonLoadTestFrame->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &VideoBoxToolbar::OnLoadTestFrame, this);
}


VideoBoxToolbar::~VideoBoxToolbar()
{
}

void VideoBoxToolbar::OnReadPacket(wxCommandEvent& event)
{
   uint8_t buf[2 * 1024];
   size_t out = wxGetApp().m_bufStream->Read(buf, sizeof(buf));
   LOGINFO("Readed %u from buffer. End %d.", out, wxGetApp().m_bufStream->GetCurrentReadPositon());
}

void VideoBoxToolbar::OnSeekButton(wxCommandEvent& event)
{
   int val = m_msInput->GetValue();
   int64_t out = wxGetApp().m_bufStream->SeekTime(val);
   LOGINFO("Seek returned %d.", out);
}

void VideoBoxToolbar::OnLoadTestFrame(wxCommandEvent& event)
{
   YV12Image image;

   m_context->frameRenderer->GetImage(&image);
   auto_buffer buffer;

   CFile file;
   file.LoadFile("special://app/data/frame.yyy", buffer);
   uint8_t *s = (uint8_t*)buffer.get();
   memcpy(image.plane[0], s, image.width * image.bpp * image.height);

   file.Close();
   buffer.clear();

   file.LoadFile("special://app/data/frame.uuu", buffer);
   uint8_t *d = image.plane[1];
   s = (uint8_t*)buffer.get();
   int w = (image.width >> image.cshift_x) * image.bpp;
   int h = (image.height >> image.cshift_y);

   for (int i = 0; i < h; i++) {
      memcpy(d, s, w);
      s += w;
      d += image.stride[1];
   }

   file.Close();
   buffer.clear();

   file.LoadFile("special://app/data/frame.vvv", buffer);
   d = image.plane[2];
   s = (uint8_t*)buffer.get();
   w = (image.width >> image.cshift_x) * image.bpp;
   h = (image.height >> image.cshift_y);

   for (int i = 0; i < h; i++) {
      memcpy(d, s, w);
      s += w;
      d += image.stride[2];
   }

   file.Close();
   buffer.clear();


   m_context->frameRenderer->Flip(AUTOSOURCE);
}
