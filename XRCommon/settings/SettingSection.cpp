#include "stdafx.h"
#include "SettingSection.h"
#include <tinyxml2.h>
#include "log/Log.h"
#include "CSetting.h"
#include "SettingManager.h"

using namespace tinyxml2;


CSettingSection::CSettingSection(const std::string &id, CSettingManager *settingsManager)
	: CSetting(id, settingsManager)
{
}


CSettingSection::~CSettingSection()
{
	for (auto& autoref : m_settings)
		delete autoref;

	m_settings.clear();
}

bool CSettingSection::Deserialize(const XMLNode *node, bool update /*= false*/)
{
	//We get our ID of section
	if (!CSetting::Deserialize(node, update))
		return false;
	const XMLElement* settingElement = node->FirstChildElement("setting");
	while(settingElement != NULL) {
		std::string settingID;
		if (DeserializeIdentification(settingElement, settingID)) {
			CSetting* setting = nullptr;

			for (auto& aref : m_settings) {
				if (aref->GetID() == settingID) {
					setting = aref;
					break;
				}
			}
			update = (setting != nullptr);
			if (!update) {
				const char* settingType = settingElement->Attribute("type");
				if (settingType == nullptr && strlen(settingType) <= 0) {
					LOGERR("Unable to read setting type of \"%s\"", settingID.c_str());
					return false;
				}
				setting = m_settingmanager->CreateSetting(GetTypeFromString(settingType), settingID, m_settingmanager);
				if (setting == nullptr) {
					LOGERR("CSettingGroup: unknown setting type \"%s\" of \"%s\"", settingType, settingID.c_str());
				}
			}
			if (setting == nullptr) {
				LOGERR("CSettingGroup: unable to create new setting \"%s\"", settingID.c_str());
			}
			else if (!setting->Deserialize(settingElement, update)) {
				LOGWARN("CSettingGroup: unable to read setting \"%s\"", settingID.c_str());
				if (!update)
					delete setting;
			}
			else if (!update)
				m_settings.push_back(setting);
		}
		settingElement = settingElement->NextSiblingElement("setting");
	}

	return true;
}

void CSettingSection::AddSetting(CSetting *setting)
{
	if (setting != nullptr)
		m_settings.push_back(setting);
}

void CSettingSection::AddSettings(const SettingList &settings)
{
	for (auto& autovar: settings) {
		m_settings.push_back(autovar);
	}
}
