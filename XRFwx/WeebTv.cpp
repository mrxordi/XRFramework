#include "stdafxf.h"
#include "WeebTv.h"
#include "main.h"
#include "XRCommon/settings/AppSettings.h"
#include "XRCommon/utils/URL.h"
#include "XRCommon/utils/StringConverter.h"
#include "XRCommon/utils/StringUtils.h"


CWeebTv::CWeebTv()
{
	CAppSettings& pSettings = GetAppSettings();
	std::set<std::string> settingSet;
	settingSet.insert("weebtv.username");
	settingSet.insert("weeb.password");
	pSettings.RegisterCallback(this, settingSet);

	Init();
}


CWeebTv::~CWeebTv()
{
// 	CAppSettings& pSettings = GetAppSettings();
// 	pSettings.UnregisterCallback(this);
}

bool CWeebTv::OnSettingChanging(const CSetting* setting)
{

	return true;
}

void CWeebTv::OnSettingChanged(const CSetting* setting)
{
	m_channelList.clear();
	Init();
}

bool CWeebTv::ParseChannelList(std::string& htmlResponse)
{
	Json::Reader m_jsonreader;
	Json::Value root;
	m_jsonreader.parse(htmlResponse, root);
	std::vector<std::string> names = root.getMemberNames();
	
	
	for (int i = 0; i < names.size(); i++)
	{
		//std::map<std::string, channel>::iterator iterator;
		Json::Value& value = root[names[i]];
		channel tmpChannel;
		std::string id = value["channel_name"].asString();


		std::pair< ChannelMap::iterator , bool > tmpIt = m_channelList.insert(make_pair(id, tmpChannel));
		
		ChannelMap::iterator it = tmpIt.first;
		(*it).second.id_name = id;
		(*it).second.cid = value["cid"].asString();
		std::string in = value["channel_description"].asString();
		
		StringConverter::Utf8ToWin1250(in, (*it).second.dectription);
		StringConverter::Utf8ToWin1250(value["channel_title"].asString(), (*it).second.title);
		//(*it).second.dectription = value["channel_description"].asString();
		//(*it).second.title = value["channel_title"].asString();
		(*it).second.img_url = value["channel_logo_url"].asString();
		(*it).second.online = std::atoi(value["channel_online"].asCString());
		(*it).second.multibitrate = (value["multibitrate"].asString() == "1") ? true : false;
		(*it).second.hasImage = (value["multibitrate"].asCString() == "1") ? true : false;
      (*it).second.handler = this;
	}


	return true;
}

bool CWeebTv::Init()
{
	CAppSettings& pSettings = GetAppSettings();

	m_username = pSettings.GetString("weebtv.username");
	m_password = pSettings.GetString("weebtv.password");
	CURL url("http://weeb.tv/api/getChannelList&option=online-now-viewed");

	std::string html;
	m_connection.SetUserAgent("XBMC");
	m_connection.SetRequestHeader("ContentType", "application/x-www-form-urlencoded");
	m_connection.SetPostData(StringUtils::Format("&username=%s&userpassword=%s", m_username.c_str(), m_password.c_str()));
	if (m_connection.Open(url)){
      m_connection.ReadData(html);
      m_connection.Close();
		return ParseChannelList(html);
	}

	return false;
}

std::string CWeebTv::GetStreamLink(int id, bool hd)
{
	ChannelMap::iterator it = m_channelList.begin();
	for (int i = 0; it != m_channelList.end(); i++) {
		it++;
		if (i == id)
			break;
	}

	if (it == m_channelList.end()) {
		LOGERR("Failed to get stream link by int id: %i - id is to high.", id);
		return "";
	}

	if (!m_channelList.empty() && m_channelList.size() > id)
		return GetStreamLink(it->first, hd);
   return "";
}

std::string CWeebTv::GetStreamLink(const std::string& str_id, bool hd)
{
	ChannelMap::iterator it = m_channelList.find(str_id);

	if (it == m_channelList.end())
	{
		LOGERR("Failed to get stream ling by string id: %s - id doesnt exist.", str_id);
		return "";
	}
	CURL url("http://weeb.tv/api/setPlayer");
	std::string html;
	m_connection.SetUserAgent("XBMC");
	m_connection.SetPostData(StringUtils::Format("&platform=XBMC&channel=%s&username=%s&userpassword=%s", it->first.c_str(), m_username.c_str(), m_password.c_str()));
	m_connection.Open(url);
	m_connection.ReadData(html);

	html = CURL::Decode(html);

	std::string rtmpLink, playPath, token, bitrate;
	std::vector<std::string> params = StringUtils::Split(html, "&");
	if (params.size() < 2 || params.at(0).empty() || params.at(1).empty())
	{
		LOGERR("Unable to parse url from the server - %s", html.c_str());
		return "";
	}

	for (int i = 0; i < params.size(); i++)
	{
		std::vector<std::string> splitparam = StringUtils::Split(params[i], "=");
		if (splitparam.size() < 2 || splitparam.at(0).empty() || splitparam.at(1).empty())
		{
			LOGERR("Unable to parse url from the server - %s", html.c_str());
			return "";
		}
		
		if (splitparam[0] == "10")
			rtmpLink = splitparam[1];
		else if (splitparam[0] == "11")
			playPath = splitparam[1];
		else if (splitparam[0] == "73")
			token = splitparam[1];
		else if (splitparam[0] == "20")
			bitrate = splitparam[1];

	}
	if (bitrate == "1" && hd)
		playPath += "HI";
	else if (bitrate == "2" && !hd)
		playPath += "LOW";

	html = rtmpLink + '/';
	html += playPath + " live=true pageUrl=token swfUrl=";
	html += token;

   m_connection.Close();

	return html;
}

const CWeebTv::channel* const CWeebTv::GetChannel(const std::string& str_id)
{

   ChannelMap::iterator it = m_channelList.find(str_id);

   if (it == m_channelList.end())
   {
      LOGERR("Failed to get stream ling by string id: %s - id doesnt exist.", str_id);
      return nullptr;
   }
   return &it->second;
}
