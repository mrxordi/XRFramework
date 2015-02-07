#pragma once

//Forward declarations
class CSettingManager;
class CSetting;
enum class SettingType;

 /*!*\class ISettingCreator
 * \brief Interface for Setting Creation
 * \author XoRdi
 * \date styczeñ 2015*/
class ISettingCreator
{
public:
	virtual CSetting* CreateSetting(SettingType settingtype, const std::string &settingId, CSettingManager* settingManaget = nullptr) const = 0;
};

