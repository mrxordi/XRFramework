#include "stdafx.h"
#include "AppSettings.h"
#include "utils/XMLFile.h"
#include "utils/XRect.h"
#include "log/Log.h"
#include "../XRFramework/filesystem/File.h"

CAppSettings* Singleton<CAppSettings>::ms_Singleton = nullptr;

CAppSettings::CAppSettings() : m_bInitialized(false)
{
	m_manager = std::make_unique<CSettingManager>();
}

CAppSettings::~CAppSettings()
{
}

bool CAppSettings::Initialize(std::string& filename)
{
	m_filename = filename;
	CXMLFile xmlDoc;
	if (!xmlDoc.LoadFile(filename))
	{
		LOGERR("Error loading settings definition from %s.", filename.c_str())
	}

	XMLElement* root = xmlDoc.RootElement();

	if (root == NULL)
		return false;

	if (!m_manager->Initialize(root))
		return false;
	else
		m_bInitialized = true;

	LOGINFO("Loaded settings from %s", m_filename.c_str());

	m_brightness = m_manager->GetFloat("rendering.brightness");
	m_contrast = m_manager->GetFloat("rendering.contrast");

	return true;
}

bool CAppSettings::SaveSettings()
{
	CXMLFile xmlDoc;
	tinyxml2::XMLElement* rootEle = xmlDoc.NewElement("settings");
	XMLNode* root = xmlDoc.InsertEndChild(rootEle);

	m_manager->Save(root);
	if (xmlDoc.SaveFile(m_filename))
		LOGDEBUG("Saved settings to: %s", m_filename.c_str());

	return true;

}

int CAppSettings::GetInt(const std::string& id)
{
	return m_manager->GetInt(id);
}

float CAppSettings::GetFloat(const std::string& id)
{
	return m_manager->GetFloat(id);
}

bool CAppSettings::GetBool(const std::string& id)
{
	return m_manager->GetBool(id);
}

std::string CAppSettings::GetString(const std::string& id)
{
	return m_manager->GetString(id);
}

bool CAppSettings::SetInt(const std::string& id, int value)
{
	return m_manager->SetInt(id, value);
}

bool CAppSettings::SetFloat(const std::string& id, float value)
{
	return m_manager->SetFloat(id, value);

}

bool CAppSettings::SetBool(const std::string& id, bool value)
{
	return m_manager->SetBool(id, value);
}

bool CAppSettings::SetString(const std::string& id, std::string value)
{
	return m_manager->SetString(id, value);
}

void CAppSettings::SaveCurrentRect(XRect& rect, int monitor)
{
	SetInt("window.width", rect.Width());
	SetInt("window.height", rect.Height());
	SetInt("window.monitor", monitor);
}
