#include "stdafx.h"
#include <sstream>

#include "UrlOptions.h"
#include "URL.h"
#include "utils/StringUtils.h"
#include "log/Log.h"

using namespace std;

CUrlOptions::CUrlOptions()
	: m_strLead("")
{ }

CUrlOptions::CUrlOptions(const std::string &options, const char *strLead /* = "" */)
	: m_strLead(strLead)
{
	AddOptions(options);
}

CUrlOptions::~CUrlOptions()
{ }

std::string CUrlOptions::GetOptionsString(bool withLeadingSeperator /* = false */) const
{
	std::string options;
	for (UrlOptions::const_iterator opt = m_options.begin(); opt != m_options.end(); opt++)
	{
		if (opt != m_options.begin())
			options += "&";

		options += CURL::Encode(opt->first);
		if (!opt->second.empty())
			options += "=" + CURL::Encode(opt->second.asString());
	}

	if (withLeadingSeperator && !options.empty())
	{
		if (m_strLead.empty())
			options = "?" + options;
		else
			options = m_strLead + options;
	}

	return std::move(options);
}

void CUrlOptions::AddOption(const std::string &key, const char *value)
{
	if (key.empty() || value == NULL)
		return;

	return AddOption(key, string(value));
}

void CUrlOptions::AddOption(const std::string &key, const std::string &value)
{
	if (key.empty())
		return;

	m_options[key] = value;
}

void CUrlOptions::AddOption(const std::string &key, int value)
{
	if (key.empty())
		return;

	m_options[key] = value;
}

void CUrlOptions::AddOption(const std::string &key, float value)
{
	if (key.empty())
		return;

	m_options[key] = value;
}

void CUrlOptions::AddOption(const std::string &key, double value)
{
	if (key.empty())
		return;

	m_options[key] = value;
}

void CUrlOptions::AddOption(const std::string &key, bool value)
{
	if (key.empty())
		return;

	m_options[key] = value;
}

void CUrlOptions::AddOptions(const std::string &options)
{
	if (options.empty())
		return;

	string strOptions = options;

	// if matching the preset leading str, remove from options.
	if (!m_strLead.empty() && strOptions.compare(0, m_strLead.length(), m_strLead) == 0)
		strOptions.erase(0, m_strLead.length());
	else if (strOptions.at(0) == '?' || strOptions.at(0) == '#' || strOptions.at(0) == ';' || strOptions.at(0) == '|')
	{
		// remove leading ?, #, ; or | if present
		if (!m_strLead.empty())
			LOGERR("original leading str %s overrided by %c", m_strLead.c_str(), strOptions.at(0));
		m_strLead = strOptions.at(0);
		strOptions.erase(0, 1);
	}

	// split the options by & and process them one by one
	vector<string> optionList = StringUtils::Split(strOptions, "&");
	for (vector<string>::const_iterator option = optionList.begin(); option != optionList.end(); option++)
	{
		if (option->empty())
			continue;

		string key, value;

		size_t pos = option->find('=');
		key = CURL::Decode(option->substr(0, pos));
		if (pos != string::npos)
			value = CURL::Decode(option->substr(pos + 1));

		// the key cannot be empty
		if (!key.empty())
			AddOption(key, value);
	}
}

void CUrlOptions::AddOptions(const CUrlOptions &options)
{
	m_options.insert(options.m_options.begin(), options.m_options.end());
}

void CUrlOptions::RemoveOption(const std::string &key)
{
	if (key.empty())
		return;

	UrlOptions::iterator option = m_options.find(key);
	if (option != m_options.end())
		m_options.erase(option);
}

bool CUrlOptions::HasOption(const std::string &key) const
{
	if (key.empty())
		return false;

	return m_options.find(key) != m_options.end();
}

bool CUrlOptions::GetOption(const std::string &key, CVariant &value) const
{
	if (key.empty())
		return false;

	UrlOptions::const_iterator option = m_options.find(key);
	if (option == m_options.end())
		return false;

	value = option->second;
	return true;
}
