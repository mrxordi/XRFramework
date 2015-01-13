#pragma once
#include "settings/helpers/Monitors.h"
/*!
 * \class CAppSettings
 * \brief Wrapper around Settings Manager responsible for prop. configuring all callbacks
 * \ and parsing files.
 */
class CAppSettings : public CObject
{
public:
	DECLARE_DYNAMIC(CAppSettings)
public:
	CAppSettings();
	~CAppSettings();

	void Initialize();

private:
	CMonitors m_monitors;


};

