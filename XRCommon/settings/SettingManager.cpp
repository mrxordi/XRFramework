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
	switch (settingtype)
	{
	case SettingType::ST_INT:
		return new CSettingInt(settingId, const_cast<CSettingManager*>(this));
	case SettingType::ST_BOOL:
		return new CSettingBool(settingId, const_cast<CSettingManager*>(this));
	case SettingType::ST_STRING:
		return new CSettingString(settingId, const_cast<CSettingManager*>(this));
	case SettingType::ST_FLOAT:
		return new CSettingFloat(settingId, const_cast<CSettingManager*>(this));
	}

	return nullptr;
}

CSettingManager::CSettingManager()
	: m_initialized(false), m_loaded(false)
{}

CSettingManager::~CSettingManager()
{
	// first clear all registered settings handler and subsettings
	// implementations because we can't be sure that they are still valid
	m_settingCreators.clear();

	for (auto& sectionpair : m_sections) {
		if (sectionpair.second)
		{
			delete sectionpair.second;
		}
	}
	m_sections.clear();
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
			SectionMap::iterator itSection = m_sections.find(settingId);
			bool update = (itSection != m_sections.end());
			if (!update)
				section = new CSettingSection(settingId, this);
			else
				section = itSection->second;

			if (section->Deserialize(sectionNode, update))
				AddSection(section);
			else {
				LOGWARN("CSettingsManager: unable to read section \"%s\"", settingId.c_str());
				if (!update)
					delete section;
			}
		}
		sectionNode = sectionNode->NextSiblingElement("section");
	}

	m_initialized = true;
	return true;
}

bool CSettingManager::Save(XMLNode *root) const
{
	XR::CSingleLock lock2(m_settingsCritical);
	if (!m_initialized || root == NULL)
		return false;

	if (!Serialize(root))
	{	
		LOGERR("CSettingsManager: failed to save settings");
		return false;
	}

	return true;
}

void CSettingManager::SetInitialized()
{

}

void CSettingManager::AddSection(CSettingSection *section)
{
	if (section == NULL)
		return;

	m_sections[section->GetID()] = section;

	for (auto settingIt = section->GetSettings().begin(); settingIt != section->GetSettings().end(); ++settingIt) {
		const std::string& settingId = (*settingIt)->GetID();
		SettingMap::iterator setting = m_settings.find(settingId);
		if (setting == m_settings.end()) {
			Setting tmpSetting = { NULL };
			std::string newid = section->GetID() +"."+ settingId;
			std::pair<SettingMap::iterator, bool> tmpIt = m_settings.insert(make_pair(newid, tmpSetting));
			setting = tmpIt.first;
		}
		if (setting->second.setting == nullptr) {
			setting->second.setting = *settingIt;
			(*settingIt)->SetCallback(this);
		}
	}
}

void CSettingManager::RegisterCallback(ISettingCallback *callback, const std::set<std::string> &settingList)
{
	XR::CSingleLock lock(m_settingsCritical);
	if (callback == nullptr)
		return;

	for (std::set<std::string>::const_iterator settingIt = settingList.begin(); settingIt != settingList.end(); ++settingIt) {
		std::string id = *settingIt;
		StringUtils::ToLower(id);

		SettingMap::iterator setting = m_settings.find(id);
		if (setting == m_settings.end()) {
			if (m_initialized)
				continue;

			Setting tmpSetting = { NULL };
			std::pair<SettingMap::iterator, bool> tmpIt = m_settings.insert(make_pair(id, tmpSetting));
			setting = tmpIt.first;
		}

		setting->second.callbacks.insert(callback);
	}
}

void CSettingManager::UnregisterCallback(ISettingCallback *callback)
{
	if (callback == nullptr)
		return;
	XR::CSingleLock lock(m_settingsCritical);
	for (auto& it : m_settings) {
		it.second.callbacks.erase(callback);
	}
}

CSetting* CSettingManager::GetSetting(const std::string &id) const
{
	XR::CSingleLock lock(m_settingsCritical);
	if (id.empty())
		return NULL;

	std::string settingId = id;
	StringUtils::ToLower(settingId);

	SettingMap::const_iterator setting = m_settings.find(settingId);
	if (setting != m_settings.end())
		return setting->second.setting;

	LOGDEBUG("CSettingsManager: requested setting (%s) was not found.", id.c_str());

	return nullptr;
}

 std::vector<CSettingSection*> CSettingManager::GetSections() const
 {
	 XR::CSingleLock lock(m_critical);
	 std::vector<CSettingSection*> sections;
	 for (SectionMap::const_iterator sectionIt = m_sections.begin(); sectionIt != m_sections.end(); ++sectionIt)
		 sections.push_back(sectionIt->second);

	 return sections;
 }

CSettingSection* CSettingManager::GetSection(const std::string &section) const
{
	XR::CSingleLock lock(m_critical);
	if (section.empty())
		return NULL;

	std::string sectionId = section;
	StringUtils::ToLower(sectionId);

	SectionMap::const_iterator sectionIt = m_sections.find(sectionId);
	if (sectionIt != m_sections.end())
		return sectionIt->second;

	LOGDEBUG("CSettingsManager: requested setting section (%s) was not found.", section.c_str());

	return nullptr;
}

bool CSettingManager::GetBool(const std::string &id) const
{
	XR::CSingleLock lock(m_settingsCritical);
	CSetting* setting = GetSetting(id);
	if (!setting) {
		LOGERR("GetBool() - Setting not found! - use AddBool() to add setting first");
		return false;
	}

	if (setting->GetType() != SettingType::ST_BOOL)
		return false;

	return ((CSettingIpml<bool>*)setting)->GetValue();
}

int CSettingManager::GetInt(const std::string &id) const
{
	XR::CSingleLock lock(m_settingsCritical);
	CSetting *setting = GetSetting(id);
	if (!setting) {
		LOGERR("GetInt() - Setting not found! - use AddInt() to add setting first");
		return 0;
	}
	if (setting->GetType() != SettingType::ST_INT)
		return false;

	return ((CSettingInt*)setting)->GetValue();
}

float CSettingManager::GetFloat(const std::string &id) const
{
	XR::CSingleLock lock(m_settingsCritical);
	CSetting *setting = GetSetting(id);
	if (!setting) {
		LOGERR("GetFloat() - Setting not found! - use AddFloat() to add setting first");
		return false;
	}
	if (setting->GetType() != SettingType::ST_FLOAT)
		return false;

	return ((CSettingFloat*)setting)->GetValue();
}

std::string CSettingManager::GetString(const std::string &id) const
{
	XR::CSingleLock lock(m_settingsCritical);
	CSetting *setting = GetSetting(id);
	if (!setting) {
		LOGERR("GetString() - Setting not found! - use AddString() to add setting first");
		return false;
	}
	if (setting->GetType() != SettingType::ST_STRING)
		return false;

	return ((CSettingString*)setting)->GetValue();
}

std::vector< std::shared_ptr<CSetting> > CSettingManager::GetList(const std::string &id) const
{
	return std::vector<std::shared_ptr<CSetting>>();
}

bool CSettingManager::SetBool(const std::string &id, bool value)
{
	XR::CSingleLock lock(m_settingsCritical);
	CSetting *setting = GetSetting(id);
	if (!setting) {
		LOGERR("SetBool() - Setting not found! - use AddSetting() to add setting first");
		return false;
	}

	if (setting->GetType() != SettingType::ST_BOOL)
		return false;

	return ((CSettingBool*)setting)->SetValue(value);
}

bool CSettingManager::ToggleBool(const std::string &id)
{
	XR::CSingleLock lock(m_settingsCritical);
	CSetting *setting = GetSetting(id);
	if (!setting) {
		LOGERR("ToggleBool() - Setting not found! - use AddSetting() to add setting first");
		return false;
	}

	if (setting->GetType() != SettingType::ST_BOOL)
		return false;
	bool value = ((CSettingBool*)setting)->GetValue();

	return ((CSettingBool*)setting)->SetValue(!value);
}

bool CSettingManager::SetInt(const std::string &id, int value)
{
	XR::CSingleLock lock(m_settingsCritical);
	CSetting *setting = GetSetting(id);
	if (!setting) {
		LOGERR("SetInt() - Setting not found! - use AddInt() to add setting first");
		return false;
	}
	if (setting->GetType() != SettingType::ST_INT)
		return false;

	return ((CSettingInt*)setting)->SetValue(value);
}

bool CSettingManager::SetFloat(const std::string &id, float value)
{
	XR::CSingleLock lock(m_settingsCritical);
	CSetting *setting = GetSetting(id);
	if (!setting) {
		LOGERR("SetFloat() - Setting not found! - use AddFloat() to add setting first");
		return false;
	}
	if (setting->GetType() != SettingType::ST_FLOAT)
		return false;

	return ((CSettingFloat*)setting)->SetValue(value);
}

bool CSettingManager::SetString(const std::string &id, const std::string &value)
{
	XR::CSingleLock lock(m_settingsCritical);
	CSetting *setting = GetSetting(id);
	if (!setting) {
		LOGERR("SetString() - Setting not found! - use AddString() to add setting first");
		return false;
	}

	if (setting->GetType() != SettingType::ST_STRING)
		return false;

	return ((CSettingString*)setting)->SetValue(value);
}

bool CSettingManager::OnSettingChanging(const CSetting *setting)
{
	if (setting == NULL)
		return false;

	XR::CSingleLock lock(m_settingsCritical);
	if (!m_loaded)
		return true;

	SettingMap::const_iterator settingIt = m_settings.find(setting->GetID());
	if (settingIt == m_settings.end())
		return false;

	Setting settingData = settingIt->second;
	// now that we have a copy of the setting's data, we can leave the lock
	lock.Leave();

	for (auto it : settingData.callbacks) {
		if (!(*it).OnSettingChanging(setting))
			return false;
	}
	return true;
}

void CSettingManager::OnSettingChanged(const CSetting *setting)
{
	XR::CSingleLock lock(m_settingsCritical);
	if (!m_loaded || setting == nullptr)
		return;

	SettingMap::const_iterator settingIt = m_settings.find(setting->GetID());
	if (settingIt == m_settings.end())
		return;

	Setting settingData = settingIt->second;
	// now that we have a copy of the setting's data, we can leave the lock
	lock.Leave();

	for (CallbackSet::iterator callback = settingData.callbacks.begin();
		callback != settingData.callbacks.end(); ++callback)
	{
		(*callback)->OnSettingChanged(setting);
	}	
}

bool CSettingManager::Serialize(XMLNode *parent) const
{
	XR::CSingleLock lock(m_settingsCritical);

	tinyxml2::XMLDocument* doc =  parent->GetDocument();

	for (auto& section : m_sections) {
		tinyxml2::XMLElement* newelementsection = doc->NewElement("section");
		newelementsection->SetAttribute("id", section.first.c_str());
		tinyxml2::XMLNode* newnodesection = parent->InsertEndChild(newelementsection);
		for (auto& setting : section.second->GetSettings())
		{
			tinyxml2::XMLElement* newelement = doc->NewElement("setting");
			newelement->SetAttribute("id", setting->GetID().c_str());
			setting->Serialize(newelement);
			newnodesection->InsertEndChild(newelement);
		}
	}
	return true;
}