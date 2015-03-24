#pragma once
#include <set>
#include <map>
#include <tinyxml2.h>
#include "CSetting.h"
#include "ISettingCallback.h"
#include "Setting.h"
#include "ISettingCreator.h"
#include "SettingSection.h"
using namespace tinyxml2;

class CSettingManager : private ISettingCreator, private ISettingCallback
{
public:
	/*!\brief Creates a new (uninitialized) settings manager.*/
	CSettingManager();
	virtual ~CSettingManager();

	// implementation of ISettingCreator
	virtual CSetting* CreateSetting(SettingType type, const std::string &settingId, 
									CSettingManager *settingsManager = nullptr) const;
	/*!\brief Initializes the settings manager using the setting definitions
	represented by the given XML element.
	\param root XML element representing setting definitions
	\return True if the XML element was successfully deserialized into setting definitions, false otherwise*/
	bool Initialize(const XMLElement *root);
	/*!\brief Saves the setting values to the given XML node.
	\param root XML node
	\return True if the setting values were successfully saved, false otherwise*/
	virtual bool Save(XMLNode *root) const;
	/*!\brief Loads the setting being represented by the given XML node with the
	given identifier.
	\param node XML node representing the setting to load
	\param settingId Setting identifier
	\return True if the setting was successfully loaded from the given XML node, false otherwise*/
	bool LoadSetting(const XMLNode *node, const std::string &settingId, bool &updated);
	/*!\brief Tells the settings system that the initialization is complete.
	Setting values can only be loaded after a complete and successful
	initialization of the settings system.*/
	void SetInitialized();
	/*!\brief Tells the settings system that all setting values
	\have been loaded.
	\This manual trigger is necessary to enable the ISettingCallback methods
	\being executed.*/
	void SetLoaded() { m_loaded = true; }
	void AddSection(CSettingSection *section);
	/*!\brief Registers the given ISettingCallback implementation to be triggered
	\for the given list of settings.
	\param settingsHandler ISettingsHandler implementation
	\param settingList List of settings to trigger the given ISettingCallback implementation*/
	void RegisterCallback(ISettingCallback *callback, const std::set<std::string> &settingList);
	/*!\brief Unregisters the given ISettingCallback implementation.
	\param callback ISettingCallback implementation*/
	void UnregisterCallback(ISettingCallback *callback);
	CSetting* GetSetting(const std::string &id) const;
	/*!\brief Gets the full list of setting sections.
	\return List of setting sections*/
	std::vector<CSettingSection*> GetSections() const;
	/*!\brief Gets the setting section with the given identifier.
	\param section Setting section identifier
	\return Setting section with the given identifier or NULL if the identifier is unknown*/
	CSettingSection* GetSection(const std::string &section) const;
	/*!\brief Gets the boolean value of the setting with the given identifier.
	\param id Setting identifier
	\return Boolean value of the setting with the given identifier*/
	bool GetBool(const std::string &id) const;
	/*!\brief Gets the integer value of the setting with the given identifier.
	\param id Setting identifier
	\return Integer value of the setting with the given identifier*/
	int GetInt(const std::string &id) const;
	/*!\brief Gets the real number value of the setting with the given identifier.
	\param id Setting identifier
	\return Real number value of the setting with the given identifier*/
	float GetFloat(const std::string &id) const;
	/*!\brief Gets the string value of the setting with the given identifier.
	\param id Setting identifier
	\return String value of the setting with the given identifier*/
	std::string GetString(const std::string &id) const;
	/*!\brief Gets the values of the list setting with the given identifier.
	\param id Setting identifier
	\return List of values of the setting with the given identifier*/
	std::vector< std::shared_ptr<CSetting> > GetList(const std::string &id) const;
	/*!\brief Sets the boolean value of the setting with the given identifier.
	\param id Setting identifier
	\param value Boolean value to set
	\return True if setting the value was successful, false otherwise*/
	bool SetBool(const std::string &id, bool value);
	/*!\brief Toggles the boolean value of the setting with the given identifier.
	\param id Setting identifier
	\return True if toggling the boolean value was successful, false otherwise*/
	bool ToggleBool(const std::string &id);
	/*!\brief Sets the integer value of the setting with the given identifier.
	\param id Setting identifier
	\param value Integer value to set
	\return True if setting the value was successful, false otherwise*/
	bool SetInt(const std::string &id, int value);
	/*!\brief Sets the real number value of the setting with the given identifier.
	\param id Setting identifier
	\param value Real number value to set
	\return True if setting the value was successful, false otherwise*/
	bool SetFloat(const std::string &id, float value);
	/*!\brief Sets the string value of the setting with the given identifier.
	\param id Setting identifier
	\param value String value to set
	\return True if setting the value was successful, false otherwise*/
	bool SetString(const std::string &id, const std::string &value);
private:
	// implementation of ISettingCallback
	virtual bool OnSettingChanging(const CSetting *setting);
	virtual void OnSettingChanged(const CSetting *setting);

	// implementation of ISubSettings
	bool Serialize(XMLNode *parent) const;

private:
	// Typedefs for our private variables
	typedef std::set<ISettingCallback*> CallbackSet;
	typedef struct {
		CSetting* setting;
		std::set<std::string> children;
		CallbackSet	callbacks;
	} Setting;

	typedef std::map<std::string, Setting> SettingMap;
	typedef std::map<std::string, CSettingSection*> SectionMap;
	typedef std::map<std::string, ISettingCreator*> SettingCreatorMap;

private:
	SettingMap m_settings;
	SectionMap m_sections;
	SettingCreatorMap m_settingCreators;

	bool m_initialized;
	bool m_loaded;

	XR::CCriticalSection m_critical;
	XR::CCriticalSection m_settingsCritical;
};