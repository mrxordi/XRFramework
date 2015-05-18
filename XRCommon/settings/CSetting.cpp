#include "stdafx.h"
#include "CSetting.h"
#include "utils/StringUtils.h"
#include "utils/URL.h"

CSetting::CSetting(const std::string &id, CSettingManager *settingsManager /*= nullptr*/)
	: m_id(id), m_settingmanager(settingsManager), m_type(SettingType::ST_UKNOWN), m_changed(false), m_callback((ISettingCallback*)m_settingmanager), m_bList(false)
{

}

bool CSetting::OnSettingChanging(const CSetting* setting)
{
	if (!m_callback)
		return true;

	return m_callback->OnSettingChanging(setting);
}

void CSetting::OnSettingChanged(const CSetting* setting)
{
	if (!m_callback)
		return;
	return m_callback->OnSettingChanged(setting);
}

bool CSetting::DeserializeIdentification(const XMLNode *node, std::string &identification)
{
	if (!node)
		return false;
	const XMLElement* element = node->ToElement();

	if (!element)
		return false;

	const char* idAttribute = element->FindAttribute("id")->Value();	//get the id of setting

	if (idAttribute == NULL || strlen(idAttribute) <= 0)
		return false;

	identification = idAttribute;
	return true;
}

bool CSetting::GetInt(const XMLNode* pRootNode, const char* strTag, int& iIntValue)
{
	const XMLElement* pNode = pRootNode->FirstChildElement(strTag);
	if (!pNode) return false;
	if (pNode->QueryIntText(&iIntValue) != XML_NO_ERROR)
		return false;

	return true;
}

bool CSetting::GetBool(const XMLNode* pRootNode, const char* strTag, bool& bBoolValue)
{
	const XMLElement* pNode = pRootNode->FirstChildElement(strTag);
	if (!pNode) return false;
	std::string strEnabled = pNode->GetText();
	StringUtils::ToLower(strEnabled);
	if (strEnabled == "off" || strEnabled == "no" || strEnabled == "disabled" || strEnabled == "false" || strEnabled == "0")
		bBoolValue = false;
	else
	{
		bBoolValue = true;
		if (strEnabled != "on" && strEnabled != "yes" && strEnabled != "enabled" && strEnabled != "true")
			return false; // invalid bool switch - it's probably some other string.
	}
	return true;
}

bool CSetting::GetString(const XMLNode* pRootNode, const char* strTag, std::string& strStringValue)
{
	const XMLElement* pElement = pRootNode->FirstChildElement(strTag);
	if (!pElement) return false;
	const char* encoded = pElement->Attribute("urlencoded");
	if (pElement != NULL)
	{
		const char* value = pElement->GetText();
		strStringValue = value ? value : "";
		if (encoded && _strcmpi(encoded, "yes") == 0)
			strStringValue = CURL::Decode(strStringValue);
		return true;
	}
	strStringValue.clear();
	return true;
}

bool CSetting::GetFloat(const XMLNode* pRootNode, const char* strTag, float& fFloatValue)
{
	const XMLElement* pNode = pRootNode->FirstChildElement(strTag);
	if (!pNode) return false;
	if (pNode->QueryFloatText(&fFloatValue) != XML_SUCCESS)
		return false;
	return true;
}

void CSetting::SetString(XMLNode* pRootNode, const char *strTag, const std::string& strValue)
{
	XMLElement* pElement = pRootNode->FirstChildElement(strTag);
	if (pElement)
		pElement->SetValue(strValue.c_str());
	else {
		XMLElement* newElement = pRootNode->GetDocument()->NewElement(strTag);
		newElement->SetName(strTag);
		newElement->SetText(strValue.c_str());
		pRootNode->InsertFirstChild(newElement);
	}

}

void CSetting::SetInt(XMLNode* pRootNode, const char *strTag, int value)
{
	XMLElement* pElement = pRootNode->FirstChildElement(strTag);
	if (pElement)
		pElement->SetText(value);
	else {
		XMLElement* newElement = pRootNode->GetDocument()->NewElement(strTag);
		newElement->SetName(strTag);
		newElement->SetText(value);
		pRootNode->InsertFirstChild(newElement);
	}
}

void CSetting::SetFloat(XMLNode* pRootNode, const char *strTag, float value)
{
	XMLElement* pElement = pRootNode->FirstChildElement(strTag);
	if (pElement)
		pElement->SetText(value);
	else {
		XMLElement* newElement = pRootNode->GetDocument()->NewElement(strTag);
		newElement->SetName(strTag);
		newElement->SetText(value);
		pRootNode->InsertFirstChild(newElement);
	}
}

void CSetting::SetBoolean(XMLNode* pRootNode, const char *strTag, bool value)
{
	XMLElement* pElement = pRootNode->FirstChildElement(strTag);
	if (pElement)
		pElement->SetText(value);
	else {
		XMLElement* newElement = pRootNode->GetDocument()->NewElement(strTag);
		newElement->SetName(strTag);
		newElement->SetText(value);
		pRootNode->InsertFirstChild(newElement);
	}
}

SettingType CSetting::GetTypeFromString(const std::string str)
{
	if (str == "float")
		return SettingType::ST_FLOAT;
	else if (str == "string")
		return SettingType::ST_STRING;
	else if (str == "integer" || str == "int")
		return SettingType::ST_INT;
	else if (str == "bool")
		return SettingType::ST_BOOL;
	else
		throw std::logic_error("Uknown setting type");
}

std::string CSetting::GetStringFromType(const SettingType str)
{
	switch (str)
	{
	case SettingType::ST_INT:
		return "integer";
	case SettingType::ST_BOOL:
		return "bool";
	case SettingType::ST_STRING:
		return "string";
	case SettingType::ST_FLOAT:
		return "float";
	}
	throw std::logic_error("Uknown setting type");
}
