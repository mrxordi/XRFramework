#include "stdafxf.h"
#include "GUITextureBase.h"
#include "GUIManager.h"
#include "GUITextureManager.h"
#include "XRCommon/log/Log.h"
#include "XRCommon/utils/SpecialProtocol.h"
#include <wx/wx.h>
#include "wx/mstream.h"


CGUITextureBase::CGUITextureBase(wxDX10Display* parent, CGUIManager* manager)
   :m_guiManager(manager), m_pParent(parent)
{
   m_pixels = nullptr;
   m_bLoadedToGPU = false;
   m_format = 0;
   m_imgHeight = m_imgWidth = m_texWidth = m_texHeight = 0;
   m_hasAlpha = false;
   m_bInitialized = false;
}

CGUITextureBase::CGUITextureBase(const CGUITextureBase &right)
{

}


CGUITextureBase::~CGUITextureBase()
{
   DestroyTextureObject();
}

bool CGUITextureBase::LoadFromFile(const std::string& texturePath)
{
   m_name = texturePath;
   // Read image into memory to use our vfs
   CFile file;
   auto_buffer buf;

   if (file.LoadFile(CSpecialProtocol::TranslatePath(texturePath), buf) <= 0)
      return false;
   wxMemoryInputStream* mem = nullptr;
   mem = new wxMemoryInputStream((uint8_t*)buf.get(), buf.size());
   wxImage* img = new wxImage(*mem, wxBITMAP_TYPE_PNG);

   m_textureResource = m_guiManager->GetTextureManager()->CreateResource(m_name, m_textureResourceId);
   m_texWidth = img->GetWidth();
   m_texHeight = img->GetHeight();
   CreateTextureObject();
   Update(img->GetData(), true);
   delete mem;
   delete img;
   m_bInitialized = true;
   return true;
}

void CGUITextureBase::CreateTextureObject()
{
    m_textureResource->Create(m_texWidth, m_texHeight, D3D10_BIND_SHADER_RESOURCE, D3D10_USAGE_DYNAMIC, DXGI_FORMAT_R8G8B8A8_UNORM);
}

void CGUITextureBase::DestroyTextureObject()
{
   m_textureResource->Release();
}

void CGUITextureBase::LoadToGPU()
{
   if (!m_pixels)
   {
      return;
      //Nothing to load no pixels
   }

   if (m_textureResource->Get() == nullptr) {
      CreateTextureObject();
      if (m_textureResource->Get() == NULL)
      {
         LOGDEBUG("CGUITextureBase::CGUITextureBase: Error creating new texture for size %d x %d", m_texWidth, m_texHeight);
         return;
      }
   }
   D3D10_MAPPED_TEXTURE2D  mapped;
   if (m_textureResource->Lock(0, D3D10_MAP_WRITE_DISCARD, &mapped))
   {
      unsigned char *dst = (unsigned char *)mapped.pData;
      unsigned char *src = m_pixels;
      unsigned int dstPitch = mapped.RowPitch;
      unsigned int srcPitch = GetPitch();
      unsigned int minPitch = std::min(srcPitch, dstPitch);
      unsigned int rows = GetRows();

      if (srcPitch == dstPitch)
      {
         memcpy(dst, src, srcPitch * rows);
      }
      else
      {
         for (unsigned int y = 0; y < rows; y++)
         {
            memcpy(dst, src, minPitch);
            src += srcPitch;
            dst += dstPitch;
         }
      }

   }
   else {
      LOGERR("Failed to lock texture.");
   }
   m_textureResource->Unlock(0);

   delete[] m_pixels;
   m_bLoadedToGPU = true;
   
}

void CGUITextureBase::Allocate(unsigned int width, unsigned int height, unsigned int format)
{
   m_imgWidth  = width;
   m_imgHeight = height;
   m_format = format;

   m_texWidth = m_imgWidth;
   m_texHeight = m_imgHeight;

   SAFE_DELETE_ARRAY(m_pixels);

   if (GetPitch() * GetRows() > 0)
   {
      m_pixels = new unsigned char[GetPitch() * GetRows()];
   }
}

unsigned int CGUITextureBase::GetPitch() 
{
   switch (m_format)
   {
   case DXGI_FORMAT_R8G8B8A8_UNORM:
   default:
      return m_texWidth * 4;
   }
}

bool CGUITextureBase::LoadFromFileInMemory(unsigned char* buffer, size_t bufferSize, const std::string& mimeType, unsigned int idealWidth /*= 0*/, unsigned int idealHeight /*= 0*/)
{
   return true;
}

void CGUITextureBase::Update(const unsigned char *pixels, bool loadToGPU)
{
    if (pixels == NULL)
        return;

    if (!m_textureResource)
        return;


    Allocate(m_texWidth, m_texHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

    unsigned int srcPitch = GetPitch();
    unsigned int srcRows = GetRows();
    unsigned int dstPitch = GetPitch();
    unsigned int dstRows = GetRows();

    if (srcPitch == dstPitch)
    {
        memcpy(m_pixels, pixels, srcPitch * std::min(srcRows, dstRows));
    }

    if (loadToGPU)
        LoadToGPU();
}
