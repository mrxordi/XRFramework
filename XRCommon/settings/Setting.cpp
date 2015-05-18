#include "stdafx.h"
#include <sstream>
#include "Setting.h"
#include "log/Log.h"
#include "utils/StringUtils.h"

template<typename T>
CSettingIpml<T>::CSettingIpml(const std::string &id, CSettingManager* manager) 
	: CSetting(id, manager)
{
}

template<typename T>
CSettingIpml<T>::CSettingIpml(const std::string &id, const CSettingIpml& setting)
	: CSetting(id, setting.m_settingmanager)
{
	Copy(setting);
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

	const char* tmpstr = element->Attribute("list");
	if (!tmpstr) {
		m_bList = false;
		return true; 
	}
	std::string stringlist(tmpstr);

	if (stringlist == "true" || stringlist == "1" || stringlist == "on")
		m_bList = true;
	else
		m_bList = false;
	return true;
}

template<typename T>
void CSettingIpml<T>::Copy(const CSettingIpml& setting)
{
	m_callback = setting.m_callback;
	m_changed = setting.m_changed;
	m_settingmanager = setting.m_settingmanager;
	m_type = setting.m_type;
	m_bList = setting.m_bList;
	m_Value = setting.m_Value;
	m_DefaultValue = setting.m_DefaultValue;
}

template<typename T>
bool CSettingIpml<T>::SetValue(T value)
{
	XR::CSingleLock lock(m_critical);
	if (value == m_Value)
		return true;

	if (!CheckValidity(value))
		return false;

	T oldValue = m_Value;
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

template<typename T>
bool CSettingIpml<T>::SetDefault(T value)
{
	XR::CSingleLock lock(m_critical);

	m_DefaultValue = value;
	m_changed = m_Value != m_DefaultValue;
	return true;
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

bool CSettingInt::FromString(const std::string &value)
{
	if (value.empty())
		return false;

	char *end = NULL;
	int ivalue = (int)strtol(value.c_str(), &end, 10);
	if (end != NULL && *end != '\0')
		return false;

	SetValue(ivalue);
	return true;
}

std::string CSettingInt::ToString()
{
	std::ostringstream oss;
	oss << m_Value;

	return oss.str();
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
	if (!CSettingIpml::Deserialize(node))
		return false;

	if (!CSetting::GetInt(node, "default", m_DefaultValue))
		return false;

	if (!CSetting::GetInt(node, "value", m_Value))
		m_Value = m_DefaultValue;

	CSetting::GetInt(node, "min", m_min);
	CSetting::GetInt(node, "max", m_max);
	CSetting::GetInt(node, "step", m_step);

	if (!CheckValidity(m_Value)) {
		LOGWARN("Setting %s value (%i) out of range(%i-%i step-%i)", GetID(), m_Value, m_min, m_max, m_step);
		m_Value = m_DefaultValue;
	}
	m_changed = m_DefaultValue != m_Value;

	return true;
}

void CSettingInt::Copy(const CSettingInt& setting)
{
	CSettingIpml::Copy(setting);
	XR::CSingleLock lock(m_critical);
	m_min = setting.m_min;
	m_max = setting.m_max;
	m_step = setting.m_step;
}

bool CSettingInt::Serialize(XMLNode *node)
{
	tinyxml2::XMLDocument * doc = node->GetDocument();
	tinyxml2::XMLElement *element = node->ToElement();
	element->SetAttribute("type", GetStringFromType(m_type).c_str());
	if (m_bList)
		element->SetAttribute("list", m_bList);

	if (m_min != 0 && m_max > m_min) {
		tinyxml2::XMLElement* newmin = doc->NewElement("min");
		newmin->SetText(m_max);
		tinyxml2::XMLElement* newmax = doc->NewElement("max");
		newmax->SetText(m_min);
		element->InsertFirstChild(newmin);
		element->InsertFirstChild(newmax);
	}
	if (m_step != 0) {
		tinyxml2::XMLElement* newstep = doc->NewElement("step");
		newstep->SetText(m_step);
		element->InsertEndChild(newstep);
	}
	if (m_changed) {
		tinyxml2::XMLElement* newvalue = doc->NewElement("value");
		newvalue->SetText(m_Value);
		element->InsertFirstChild(newvalue);
	}
	tinyxml2::XMLElement* newdefault = doc->NewElement("default");
	newdefault->SetText(m_DefaultValue);
	element->InsertEndChild(newdefault);
	return true;
}

/*!
* \class CSettingFloat
*/
CSettingFloat::CSettingFloat(const std::string &id, CSettingManager *settingsManager /*= NULL*/)
	: CSettingIpml(id, settingsManager), m_min(0.0f), m_max(0.0f), m_step(1.0)
{
	m_type = SettingType::ST_FLOAT;
	m_Value = 0.0f;
	m_DefaultValue = 0.0f;
}

CSettingFloat::CSettingFloat(const std::string &id, const CSettingFloat &setting)
	: CSettingIpml(id, setting), m_min(0.0f), m_max(0.0f), m_step(1.0f)
{
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
	m_min = setting.m_min;
	m_max = setting.m_max;
	m_step = setting.m_step;
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

	if (!CSetting::GetFloat(node, "default", m_DefaultValue))
		return false;

	if (!CSetting::GetFloat(node, "value", m_Value))
		m_Value = m_DefaultValue;


	CSetting::GetFloat(node, "min", m_min);
	CSetting::GetFloat(node, "max", m_max);
	CSetting::GetFloat(node, "step", m_step);
	if (!CheckValidity(m_Value)) {
		LOGWARN("Setting %s value (%f) out of range(%f-%f step-%f)", GetID(), m_Value, m_min, m_max, m_step);
		m_Value = m_DefaultValue;
	}
	m_changed = m_DefaultValue != m_Value;

	return true;
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

bool CSettingFloat::Serialize(XMLNode *node)
{
	tinyxml2::XMLDocument * doc = node->GetDocument();
	tinyxml2::XMLElement *element = node->ToElement();
	element->SetAttribute("type", GetStringFromType(m_type).c_str());
	if (m_bList)
		element->SetAttribute("list", m_bList);

	if (m_min != 0 && m_max > m_min) {
		tinyxml2::XMLElement* newmin = doc->NewElement("min");
		newmin->SetText(m_min);
		tinyxml2::XMLElement* newmax = doc->NewElement("max");
		newmax->SetText(m_max);
		element->InsertFirstChild(newmin);
		element->InsertFirstChild(newmax);
	}
	if (m_step != 0) {
		tinyxml2::XMLElement* newstep = doc->NewElement("step");
		newstep->SetText(m_step);
		element->InsertEndChild(newstep);
	}
	if (m_changed) {
		tinyxml2::XMLElement* newvalue = doc->NewElement("value");
		newvalue->SetText(m_Value);
		element->InsertFirstChild(newvalue);
	}
	tinyxml2::XMLElement* newdefault = doc->NewElement("default");
	newdefault->SetText(m_DefaultValue);
	element->InsertEndChild(newdefault);
	return true;
}

CSettingBool::CSettingBool(const std::string &id, CSettingManager *settingsManager /*= NULL*/) 
	: CSettingIpml(id, settingsManager)
{
	m_type = SettingType::ST_BOOL;
	m_Value = false;
	m_DefaultValue = false;
}

CSettingBool::CSettingBool(const std::string &id, const CSettingBool &setting) 
	: CSettingIpml(id, setting)
{
	Copy(setting);
}

CSettingBool::CSettingBool(const std::string &id, bool value, CSettingManager *settingsManager /*= NULL*/) 
	: CSettingIpml(id, settingsManager)
{
	m_Value = value;
}

bool CSettingBool::CheckValidity(const bool &value) const
{
	return true;
}

bool CSettingBool::Deserialize(const XMLNode *node, bool update /*= false*/)
{
	XR::CSingleLock lock(m_critical);
	if (!CSettingIpml::Deserialize(node))
		return false;

	if (!CSetting::GetBool(node, "default", m_DefaultValue))
		return false;

	if (!CSetting::GetBool(node, "value", m_Value))
		m_Value = m_DefaultValue;

	m_changed = m_Value != m_DefaultValue;

	return true;
}

bool CSettingBool::FromString(const std::string &value)
{
	if (StringUtils::EqualsNoCase(value, "true"))
	{
		SetValue(true);
		return true;
	}
	if (StringUtils::EqualsNoCase(value, "false"))
	{
		SetValue(false);
		return true;
	}

	return false;
}

std::string CSettingBool::ToString()
{
	return m_Value ? "true" : "false";
}

void CSettingBool::Copy(const CSettingBool& setting)
{
	CSettingIpml::Copy(setting);
}

bool CSettingBool::Serialize(XMLNode *node)
{
	tinyxml2::XMLDocument * doc = node->GetDocument();
	tinyxml2::XMLElement *element = node->ToElement();
	element->SetAttribute("type", GetStringFromType(m_type).c_str());
	if (m_bList)
		element->SetAttribute("list", m_bList);

	if (m_changed) {
		tinyxml2::XMLElement* newvalue = doc->NewElement("value");
		newvalue->SetText(m_Value);
		element->InsertFirstChild(newvalue);
	}
	tinyxml2::XMLElement* newdefault = doc->NewElement("default");
	newdefault->SetText(m_DefaultValue);
	element->InsertEndChild(newdefault);
	return true;
}

CSettingString::CSettingString(const std::string &id, CSettingManager *settingsManager /*= NULL*/) 
	: CSettingIpml(id, settingsManager)
{
	m_type = SettingType::ST_STRING;
	m_Value = "";
	m_DefaultValue = "";
}

CSettingString::CSettingString(const std::string &id, const CSettingString &setting)
	: CSettingIpml(id, setting.m_settingmanager)
{
	Copy(setting);
}

CSettingString::CSettingString(const std::string &id, std::string value, CSettingManager *settingsManager /*= NULL*/)
	: CSettingIpml(id, m_settingmanager)
{
	m_Value = value;
	m_DefaultValue = value;
}

bool CSettingString::CheckValidity(const std::string &value) const
{
	return !value.empty();
}

bool CSettingString::Deserialize(const XMLNode *node, bool update /*= false*/)
{
	XR::CSingleLock lock(m_critical);
	if (!CSettingIpml::Deserialize(node, update))
		return false;

	if (!CSetting::GetString(node, "default", m_DefaultValue))
		return false;

	if (!CSetting::GetString(node, "value", m_Value))
		m_Value = m_DefaultValue;

	m_changed = m_DefaultValue != m_Value;

	if (!CheckValidity(m_Value)) {
		LOGWARN("Setting %s value is empty (setting default value).", GetID());
		m_Value = m_DefaultValue;
	}
	m_changed = m_DefaultValue != m_Value;

	return true;
}

bool CSettingString::FromString(const std::string &value)
{
	return CSettingIpml::SetValue(value);
}

std::string CSettingString::ToString()
{
	return std::string(m_Value);
}

void CSettingString::Copy(const CSettingString& setting)
{
	CSettingIpml::Copy(setting);
}

bool CSettingString::Serialize(XMLNode *node)
{
	tinyxml2::XMLDocument * doc = node->GetDocument();
	tinyxml2::XMLElement *element = node->ToElement();
	element->SetAttribute("type", GetStringFromType(m_type).c_str());
	if (m_bList)
		element->SetAttribute("list", m_bList);
	if (m_changed) {
		tinyxml2::XMLElement* newvalue = doc->NewElement("value");
		newvalue->SetText(m_Value.c_str());
		element->InsertFirstChild(newvalue);
	}
	tinyxml2::XMLElement* newdefault = doc->NewElement("default");
	newdefault->SetText(m_DefaultValue.c_str());
	element->InsertEndChild(newdefault);
	return true;
}
