#pragma once
#include "settings/helpers/Monitors.h"
#include "settings/SettingManager.h"
#include "utils/Singleton.h"

class XRect;

/*!
 * \class CAppSettings
 * \brief Wrapper around Settings Manager responsible for prop. configuring all callbacks
 * \ and parsing files.
 */
class CAppSettings : public Singleton<CAppSettings>	
{
public:
public:
	CAppSettings();
	~CAppSettings();

	bool Initialize(std::string& filename);
	bool SaveSettings();

	void SaveCurrentRect(XRect& rect, int monitor = 0);
public:
	int GetInt(const std::string& id);
	float GetFloat(const std::string& id);
	bool GetBool(const std::string& id);
	std::string GetString(const std::string& id);

	bool SetInt(const std::string& id, int value);
	bool SetFloat(const std::string& id, float value);
	bool SetBool(const std::string& id, bool value);
	bool SetString(const std::string& id, std::string value);

	/*!\brief Registers the given ISettingCallback implementation to be triggered
	\for the given list of settings.
	\param settingsHandler ISettingsHandler implementation
	\param settingList List of settings to trigger the given ISettingCallback implementation*/
	void RegisterCallback(ISettingCallback *callback, const std::set<std::string> &settingList);
	/*!\brief Unregisters the given ISettingCallback implementation.
	\param callback ISettingCallback implementation*/
	void UnregisterCallback(ISettingCallback *callback);

public:
	float m_contrast;
	float m_brightness;
private:
	std::unique_ptr<CSettingManager> m_manager;
	bool m_bInitialized;
	std::string m_filename;
};

#define g_AppSettings CAppSettings::getSingletonPtr()
