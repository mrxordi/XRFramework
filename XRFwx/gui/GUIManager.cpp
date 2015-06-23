#include "stdafxf.h"
#include "gui/GUIManager.h"
#include "gui/GUITextureManager.h"


CGUIManager::CGUIManager(wxDX10Display* parent) : m_pParent(parent)
{
   m_pTextureManager = std::make_unique<CGUITextureManager>(m_pParent, this);
}


CGUIManager::~CGUIManager()
{

}

bool CGUIManager::Initialize()
{

   return true;
}

