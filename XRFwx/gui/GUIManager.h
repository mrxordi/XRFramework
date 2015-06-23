#pragma once
#include <memory>
#include "Context.h"
#include "wxDX10Display.h"
#include "GUITextureManager.h"
class wxDX10Display;

class CGUIManager
{
public:
   CGUIManager(wxDX10Display* ctx);
   virtual ~CGUIManager();
   virtual bool Initialize();


   CGUITextureManager* GetTextureManager() { if (m_pTextureManager) return m_pTextureManager.get(); return nullptr; };
private:
   wxDX10Display* m_pParent;
   std::unique_ptr<CGUITextureManager> m_pTextureManager;
};

