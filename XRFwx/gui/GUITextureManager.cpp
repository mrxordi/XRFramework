#include "stdafxf.h"
#include "GUIManager.h"
#include "GUITextureManager.h"
#include "XRThreads/SingleLock.h"
#include "XRFramework/render/RenderSystemDX.h"
#include "XRFramework/filesystem/File.h"
#include "XRCommon/Util.h"
#include "XRCommon/log/Log.h"

CGUITextureManager::CGUITextureManager(wxDX10Display* parent, CGUIManager* manager) 
   :m_guiManager(manager), m_pParent(parent)
{
}

CGUITextureManager::~CGUITextureManager()
{
    std::map<std::string, CGUITextureBase*>::iterator it = m_guiTextures.begin();
    while (it != m_guiTextures.end())
    {
        SAFE_DELETE(it->second);
        ++it;
    }
}


std::shared_ptr<D3DTexture> CGUITextureManager::CreateResource(std::string resName, uint32_t& id)
{
   std::shared_ptr<D3DTexture> ReturnValue;
   if (resName == "")
      return std::move(ReturnValue);

   std::map<std::string, CGUITextureBase*>::iterator it = m_guiTextures.begin();
   while ( it != m_guiTextures.end() )
   {
      if (it->first == resName)
      {
         ReturnValue = GetResource(it->second->GetResourceId());
         LOGDEBUG("Resource already loaded with ID:%u... References: %u", it->second->GetResourceId(), ReturnValue.use_count());
         std::pair<ResourceMap::iterator, bool> tmpIt = m_textureResources.insert(make_pair(m_lastId, std::move(ReturnValue)));
         m_lastId++;
         return tmpIt.first->second;
      }
      ++it;
   }
   LOGDEBUG("Resource not found.. Creating new resource.");

   ReturnValue = std::make_shared<D3DTexture>(m_pParent->GetDXRenderer());
   if (ReturnValue) {
      std::pair<ResourceMap::iterator, bool> tmpIt = m_textureResources.insert(make_pair(m_lastId, std::move(ReturnValue)));
      m_lastId++;
      return tmpIt.first->second;
   }
   return nullptr;
}

bool CGUITextureManager::DeleteResource(uint32_t id)
{
   std::map<uint32_t, std::shared_ptr<D3DTexture>>::iterator it = m_textureResources.find(id);

   if (it != m_textureResources.end ())
   {
      m_textureResources.erase(it);
      return true;
   }
   return false;
}


CGUITextureBase* CGUITextureManager::CreateTexture(const std::string& strTexturePath)
{
   if (strTexturePath.empty())
      return nullptr;
   CGUITextureBase* retVal = new CGUITextureBase(m_pParent, m_guiManager);
   
   m_guiTextures.insert(make_pair(strTexturePath, retVal));
   return retVal;
}

std::string CGUITextureManager::GetTexturePath(const std::string &textureName)
{
   if (CFile::Exists(textureName))
      return textureName;
   LOGFATAL("Texture File not found - %s.", textureName.c_str());
   return "";
}

void CGUITextureManager::DeleteTexture(CGUITextureBase* ptr)
{
   std::map<std::string, CGUITextureBase*>::iterator it = m_guiTextures.begin();
   while (it != m_guiTextures.end())
   {
      if (it->second == ptr) {
         m_guiTextures.erase(it);
         return;
      }
   }
}
