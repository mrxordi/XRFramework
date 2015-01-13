#pragma once
#include "CSetting.h"

class CSettingManager;

class CSettingSection : public CSetting
{
public:
	CSettingSection(const std::string &id, CSettingManager *settingsManager = NULL);
	~CSettingSection();

	virtual bool Deserialize(const XMLNode *node, bool update = false) override;

	const SettingList& GetSettings() const { return m_settings; }

	void AddSetting(CSetting *setting);
	void AddSettings(const SettingList &settings);

private:
	SettingList m_settings;
};

