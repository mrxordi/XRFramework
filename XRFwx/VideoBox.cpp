#include "stdafxf.h"
#include "VideoBox.h"
#include "Toolbar.h"
#include "MainFrame.h"


VideoBox::VideoBox(wxWindow *parent, Context *ctx)
   : wxPanel(parent, -1), m_context(ctx)
{
   wxToolBar* buttonToolbar = new Toolbar(this, "videonavigation", ctx, "", false);
   auto buttonSeek = new wxButton(this, IDB_Seek, "Seek To");
   auto buttonRead = new wxButton(this, IDB_Read, "Read Packet");
   auto msInput = new wxSpinCtrl(this, IDB_SecInput, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_LEFT, 0, 1000000, 0);



   auto videoBottomSizer = new wxBoxSizer(wxHORIZONTAL);
   videoBottomSizer->Add(buttonToolbar, wxSizerFlags(0).Center());
   videoBottomSizer->Add(buttonRead, wxSizerFlags(1).Center().Border(wxLEFT));
   videoBottomSizer->Add(msInput, wxSizerFlags(1).Center().Border(wxLEFT));
}


VideoBox::~VideoBox()
{
}
