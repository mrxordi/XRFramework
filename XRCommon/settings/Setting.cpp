#include "stdafx.h"
#include <sstream>
#include "Setting.h"

template<typename T>
CSettingIpml<T>::CSettingIpml(const std::string &id, CSettingManager* manager) 
	: CSetting(id, manager)
{
}

template<typename T>
CSettingIpml<T>::CSettingIpml(const std::string &id, const CSettingIpml& setting)
	: CSetting(id, setting.m_settingmanager)
{
}

template<typename T>
bool CSettingIpml<T>::Deserialize(const XMLNode *node, bool update /*= false*/)
{
	if (!CSetting::Deserialize(node, update))
		return false;

	const XMLElement *element = node->ToElement();
	if (element == NULL)
		return false;

	std::string stringtype = element->Attribute("type");
	
	m_type = GetTypeFromString(stringtype);
	
	std::string stringlist = element->Attribute("list");
	if (stringlist == "true" || stringlist == "1" || stringlist == "on")
		m_bList = true;
	else
		m_bList = false;

}

template<typename T>
void CSettingIpml<T>::Copy(const CSettingIpml& setting)
{
	m_callback = setting.m_callback;
	m_changed = setting.m_changed;
	m_settingmanager = setting.m_settingmanager;
	m_type = setting.m_type;
	m_bList = setting.m_bList;
}

/*!
* \class CSettingInt
*/
CSettingInt::CSettingInt(const std::string &id, CSettingManager *settingsManager /*= NULL*/)
	: CSettingIpml(id, settingsManager), m_min(0), m_max(0), m_step(1)
{
	m_type = SettingType::ST_INT;
	m_Value = 0;
	m_DefaultValue = 0;
}

CSettingInt::CSettingInt(const std::string &id, const CSettingInt &setting)
	: CSettingIpml(id, setting), m_min(0), m_max(0), m_step(1)
{
	m_type = SettingType::ST_INT;
	Copy(setting);
}

CSettingInt::CSettingInt(const std::string &id, int value, CSettingManager *settingsManager /*= NULL*/)
	: CSettingIpml(id, settingsManager), m_min(0), m_max(0), m_step(1)
{
	m_type = SettingType::ST_INT;
	m_Value = value;
	m_DefaultValue = 0;
}

CSettingInt::CSettingInt(const std::string &id, int value, int minimum, int step, int maximum, CSettingManager *settingsManager /*= NULL*/)
	: CSettingIpml(id, settingsManager), m_min(minimum), m_max(maximum), m_step(step)
{
	m_type = SettingType::ST_INT;
	m_Value = value;
	m_DefaultValue = 0;
}

bool CSettingInt::SetValue(int value)
{
	XR::CSingleLock lock(m_critical);

	if (value == m_Value)
		return true;

	if (!CheckValidity(value))
		return false;

	int oldValue = m_Value;
	m_Value = value;
	if (!OnSettingChanging(this)) {
		m_Value = oldValue;

		// the setting couldn't be changed because one of the
		// callback handlers failed the OnSettingChanging()
		// callback so we need to let all the callback handlers
		// know that the setting hasn't changed
		OnSettingChanging(this);
		return false;
	}

	m_changed = m_Value != m_DefaultValue;
	OnSettingChanged(this);
	return true;
}

bool CSettingInt::SetDefault(int value)
{
	XR::CSingleLock lock(m_critical);

	m_DefaultValue = value;
	if (!m_changed)
		m_Value = m_DefaultValue;

	return true;
}

bool CSettingInt::FromString(const std::string &value)
{
	if (value.empty())
		return false;

	char *end = NULL;
	m_Value = (int)strtol(value.c_str(), &end, 10);
	if (end != NULL && *end != '\0')
		return false;

	return true;
}

std::string CSettingInt::ToString()
{
	std::ostringstream oss;
	oss << m_Value;

	return oss.str();
}

bool CSettingInt::Equals(const int &value) const
{
	XR::CSingleLock lock(m_critical);
	return (m_Value == value);
}

bool CSettingInt::CheckValidity(const int &value) const
{
	if (!m_vSettingList.empty()) {
		//if the setting is an std::vector, check if we got a valid value before assigning it
		
		for (auto& autovalue : m_vSettingList) {
			if (autovalue == value)
				return true;
		}
		return false;
	}
	
	return ((m_min == m_max) || (value >= m_min && value <= m_max && ((value-m_min) % m_step == 0)));
}

bool CSettingInt::Deserialize(const XMLNode *node, bool update /*= false*/)
{
	XR::CSingleLock lock(m_critical);
	if (!CSetting::Deserialize(node))
		return false;

	int value;
	if (!CSetting::GetInt(node, "value", value))
		return false;

	if (!CSetting::GetInt(node, "default", m_DefaultValue))
		return false;

	m_changed = m_DefaultValue != value;

	CSetting::GetInt(node, "min", m_min);
	CSetting::GetInt(node, "max", m_max);
	CSetting::GetInt(node, "step", m_step);
}

void CSettingInt::Copy(const CSettingInt& setting)
{
	CSettingIpml::Copy(setting);

	XR::CSingleLock lock(m_critical);
	m_Value = setting.m_Value;
	m_DefaultValue = setting.m_DefaultValue;
	m_min = setting.m_min;
	m_max = setting.m_max;
	m_step = setting.m_step;
}

/*!
* \class CSettingFloat
*/
CSettingFloat::CSettingFloat(const std::string &id, CSettingManager *settingsManager /*= NULL*/)
	: CSettingIpml(id, settingsManager), m_min(0.0f), m_max(0.0f), m_step(1.0)
{
	m_type = SettingType::ST_FLOAT;
	m_Value = 0.0f;
}

CSettingFloat::CSettingFloat(const std::string &id, const CSettingFloat &setting)
	: CSettingIpml(id, setting), m_min(0.0f), m_max(0.0f), m_step(1.0f)
{
	m_type = SettingType::ST_FLOAT;
	Copy(setting);
}

CSettingFloat::CSettingFloat(const std::string &id, float value, CSettingManager *settingsManager /*= NULL*/)
	: CSettingIpml(id, settingsManager), m_min(0.0f), m_max(0.0f), m_step(1.0)
{
	m_type = SettingType::ST_FLOAT;
	m_Value = value;
	m_DefaultValue = 0;
}

CSettingFloat::CSettingFloat(const std::string &id, float value, float minimum, float step, float maximum, CSettingManager *settingsManager /*= NULL*/)
	: CSettingIpml(id, settingsManager), m_min(minimum), m_max(maximum), m_step(step)
{
	m_type = SettingType::ST_FLOAT;
	m_Value = value;
	m_DefaultValue = 0;
}

void CSettingFloat::Copy(const CSettingFloat& setting)
{
	CSettingIpml::Copy(setting);
	m_Value = setting.m_Value;
	m_DefaultValue = setting.m_DefaultValue;
	m_min = setting.m_min;
	m_max = setting.m_max;
	m_step = setting.m_step;
}

bool CSettingFloat::SetValue(float value)
{
	XR::CSingleLock lock(m_critical);

	if (value == m_Value)
		return true;

	if (!CheckValidity(value))
		return false;

	float oldValue = m_Value;
	m_Value = value;
	if (!OnSettingChanging(this)) {
		m_Value = oldValue;

		// the setting couldn't be changed because one of the
		// callback handlers failed the OnSettingChanging()
		// callback so we need to let all the callback handlers
		// know that the setting hasn't changed
		OnSettingChanging(this);
		return false;
	}

	m_changed = m_Value != m_DefaultValue;
	OnSettingChanged(this);
	return true;
}

bool CSettingFloat::SetDefault(float value)
{
	XR::CSingleLock lock(m_critical);

	m_DefaultValue = value;
	if (!m_changed)
		m_Value = m_DefaultValue;

	return true;
}

bool CSettingFloat::Equals(const float &value) const
{
	XR::CSingleLock lock(m_critical);
	return (m_Value == value);
}

bool CSettingFloat::CheckValidity(const float &value) const
{
	XR::CSingleLock lock(m_critical);
	if (m_min != m_max &&
		(value < m_min || value > m_max))
		return false;

	return true;
}

bool CSettingFloat::Deserialize(const XMLNode *node, bool update /*= false*/)
{
	XR::CSingleLock lock(m_critical);
	if (!CSetting::Deserialize(node))
		return false;

	float value;
	if (!CSetting::GetFloat(node, "value", value))
		m_changed = false;

	if (!CSetting::GetFloat(node, "default", m_DefaultValue))
		return false;

	m_changed = m_DefaultValue != value;

	CSetting::GetFloat(node, "min", m_min);
	CSetting::GetFloat(node, "max", m_max);
	CSetting::GetFloat(node, "step", m_step);
}

bool CSettingFloat::FromString(const std::string &value)
{
	if (value.empty())
		return false;
	float newval;
	char *end = NULL;
	newval = strtof(value.c_str(), &end);
	if (end != NULL && *end != '\0')
		return false;

	SetValue(newval);
	return true;
}

std::string CSettingFloat::ToString()
{
	std::ostringstream oss;
	oss << m_Value;

	return oss.str();
}
