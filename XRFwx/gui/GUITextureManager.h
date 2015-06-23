#pragma once
#include <memory>
#include <vector>
#include "Context.h"
#include "wxDX10Display.h"
#include "gui/GUITexture.h"
#include "XRThreads/CriticalSection.h"
class CGUIManager;
class CGUITextureBase;
class D3DTexture;
class wxDX10Display;

typedef std::map<uint32_t, std::shared_ptr<D3DTexture>> ResourceMap;
typedef std::map<std::string, CGUITextureBase*> GUITexturesMap;


class CGUITextureManager
{
public:
   CGUITextureManager(wxDX10Display* m_parent, CGUIManager* manager);
   ~CGUITextureManager();


   CGUITextureBase* CreateTexture(const std::string& strTextureName);
   void DeleteTexture(CGUITextureBase* ptr);

   bool                        DeleteResource(uint32_t id);
   std::shared_ptr<D3DTexture> CreateResource(std::string name, uint32_t& id);
   std::shared_ptr<D3DTexture> GetResource(uint32_t id) { return m_textureResources[id]; };
   std::string                 GetTexturePath(const std::string &textureName);

private:
   wxDX10Display* m_pParent;
   CGUIManager* m_guiManager;

   ResourceMap    m_textureResources;
   GUITexturesMap m_guiTextures;
   uint32_t m_lastId;
};

