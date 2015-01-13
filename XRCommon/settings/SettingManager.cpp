#include "stdafx.h"
#include "SettingManager.h"
#include "../XRThreads/SingleLock.h"
#include "utils/StringUtils.h"
#include "log/Log.h"
#include "SettingSection.h"
#include "CSetting.h"

using namespace tinyxml2;

CSetting* CSettingManager::CreateSetting(SettingType settingtype, const std::string &settingId, CSettingManager* settingManaget /*= nullptr*/) const
{
	throw std::logic_error("The method or operation is not implemented.");
}

CSettingManager::CSettingManager()
	: m_initialized(false), m_loaded(false)
{}

CSettingManager::~CSettingManager()
{
	// first clear all registered settings handler and subsettings
	// implementations because we can't be sure that they are still valid
	m_settingsHandlers.clear();
	m_subSettings.clear();
	m_settingCreators.clear();
	Clear();
}

bool CSettingManager::Initialize(const XMLElement *root)
{
	XR::CSingleLock lock(m_critical);
	XR::CSingleLock lock2(m_settingsCritical);
	if (m_initialized || root == NULL)
		return false;

	if (!StringUtils::EqualsNoCase(root->Value(), "settings"))
	{
		LOGERR("CSettingsManager: error reading settings definition: doesn't contain <settings> tag");
		return false;
	}

	const XMLElement *sectionNode = root->FirstChildElement("section");
	while (sectionNode != nullptr) {
		std::string settingId;
		if (CSetting::DeserializeIdentification(sectionNode, settingId)) {
			CSettingSection* section = nullptr;
			auto itSection = m_sections.find(settingId);
		}
	}


}

bool CSettingManager::Load(const XMLElement *root, bool &updated, bool triggerEvents /*= true*/, std::map<std::string, CSetting*> *loadedSettings /*= NULL*/)
{
	return true;
}

bool CSettingManager::Load(const XMLNode *settings)
{
	return true;

}

bool CSettingManager::Save(XMLNode *root) const
{
	return true;

}

void CSettingManager::Unload()
{

}

void CSettingManager::Clear()
{

}

bool CSettingManager::LoadSetting(const XMLNode *node, const std::string &settingId)
{
	return true;
}

bool CSettingManager::LoadSetting(const XMLNode *node, const std::string &settingId, bool &updated)
{
	return true;

}

bool CSettingManager::LoadSetting(const XMLNode *node, CSetting *setting, bool &updated)
{
	return true;

}

void CSettingManager::SetInitialized()
{

}

void CSettingManager::AddSection(CSettingSection *section)
{

}

void CSettingManager::RegisterCallback(ISettingCallback *callback, const std::set<std::string> &settingList)
{

}

void CSettingManager::UnregisterCallback(ISettingCallback *callback)
{

}

void CSettingManager::RegisterSettingType(const std::string &settingType, ISettingCreator *settingCreator)
{

}

void CSettingManager::RegisterSettingsHandler(ISettingsHandler *settingsHandler)
{

}

void CSettingManager::UnregisterSettingsHandler(ISettingsHandler *settingsHandler)
{

}

void CSettingManager::RegisterSubSettings(ISubSettings *subSettings)
{

}

void CSettingManager::UnregisterSubSettings(ISubSettings *subSettings)
{

}

CSetting* CSettingManager::GetSetting(const std::string &id) const
{
	return nullptr;
}

// std::vector<CSettingSection*> CSettingManager::GetSections() const
// {
// 
// }

CSettingSection* CSettingManager::GetSection(const std::string &section) const
{
	return nullptr;
}

bool CSettingManager::GetBool(const std::string &id) const
{
	return true;

}

int CSettingManager::GetInt(const std::string &id) const
{
	return 0;
}

double CSettingManager::GetNumber(const std::string &id) const
{
	return 0.0;
}

std::string CSettingManager::GetString(const std::string &id) const
{
	return "";
}

std::vector< boost::shared_ptr<CSetting> > CSettingManager::GetList(const std::string &id) const
{
	return std::vector<boost::shared_ptr<CSetting>>();
}

bool CSettingManager::SetBool(const std::string &id, bool value)
{
	return true;
}

bool CSettingManager::ToggleBool(const std::string &id)
{
	return true;
}

bool CSettingManager::SetInt(const std::string &id, int value)
{
	return true;
}

bool CSettingManager::SetNumber(const std::string &id, double value)
{
	return true;
}

bool CSettingManager::SetString(const std::string &id, const std::string &value)
{
	return true;
}

bool CSettingManager::OnSettingChanging(const CSetting *setting)
{
	return true;
}

void CSettingManager::OnSettingChanged(const CSetting *setting)
{

}

bool CSettingManager::OnSettingsLoading()
{
	return true;
}

void CSettingManager::OnSettingsLoaded()
{

}

void CSettingManager::OnSettingsUnloaded()
{

}

bool CSettingManager::OnSettingsSaving() const
{
	return true;
}

void CSettingManager::OnSettingsSaved() const
{

}

void CSettingManager::OnSettingsCleared()
{

}

bool CSettingManager::Serialize(XMLNode *parent) const
{
	return true;
}

bool CSettingManager::Deserialize(const XMLNode *node, bool &updated, std::map<std::string, CSetting*> *loadedSettings /*= NULL*/)
{
	return true;
}
