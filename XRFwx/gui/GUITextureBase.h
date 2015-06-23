#pragma once
#include "wxDX10Display.h"
#include "gui/GUIManager.h"
#include "gui/GUITextureManager.h"
#include "XRFramework/render/ID3DResource.h"
#include "XRFramework/utils/XRect.h"
#include "XRFramework/filesystem/File.h"
#include "XRFramework/utils/AutoBuffer.h"

class wxDX10Display;
class CGUIManager;
class CTextureInfo
{
public:
   CTextureInfo();
   CTextureInfo(const std::string &file) :
      filename(file)
   {
      orientation = 0;
      useLarge = false;
   };
   CTextureInfo& operator=(const CTextureInfo &right) {
      border = right.border;
      orientation = right.orientation;
      filename = right.filename;
      useLarge = right.useLarge;
      return *this;
   };
   bool       useLarge;
   XRect      border;          // scaled  - unneeded if we get rid of scale on load
   int        orientation;     // orientation of the texture (0 - 7 == EXIForientation - 1)
   std::string filename;        // main texture file
};

class CGUITextureBase
{
public:
   CGUITextureBase(wxDX10Display* parent, CGUIManager* manager);
   CGUITextureBase(const CGUITextureBase &left);
   virtual ~CGUITextureBase();

   void CreateTextureObject();
   void DestroyTextureObject();
   void LoadToGPU();
   void Allocate(unsigned int width, unsigned int height, unsigned int format);
   void Update(const unsigned char *pixels, bool loadToGPU);

   unsigned int GetPitch();
   unsigned int GetRows() { return m_texHeight; };
   std::string GetName() { return m_name; };


   uint32_t GetResourceId() { if (m_bInitialized) return m_textureResourceId; };

   bool LoadFromFile(const std::string& texturePath);

   bool LoadFromFileInMemory(unsigned char* buffer, size_t bufferSize, const std::string& mimeType,
                                         unsigned int idealWidth = 0, unsigned int idealHeight = 0);
   D3DTexture* GetTextureResource() { if (m_bInitialized) return m_textureResource.get(); else return nullptr; };

private:
   wxDX10Display* m_pParent;
   CGUIManager* m_guiManager;
   unsigned char* m_pixels;
   bool m_bInitialized;
   bool m_bLoadedToGPU;
   bool m_hasAlpha;
   uint32_t m_textureResourceId;
   std::shared_ptr<D3DTexture> m_textureResource;


   uint32_t m_texWidth;
   uint32_t m_texHeight;
   uint32_t m_imgWidth;
   uint32_t m_imgHeight;

   unsigned int m_format;
   std::string m_name;
};

