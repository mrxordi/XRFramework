#pragma once
#include "XRCommon/settings/ISettingCallback.h"
#include "XRCommon/json/json.h"
#include "XRFramework/filesystem/CurlFile.h"


class CWeebTv : public ISettingCallback
{
public:
	struct channel {
		std::string cid, 
			id_name,
			title,
			dectription,
			img_url;
		int online;
		bool multibitrate;
		bool hasImage;
      CWeebTv* handler;
	};

	typedef std::map <std::string, channel> ChannelMap;

public:
	CWeebTv();
	virtual		~CWeebTv();
	bool		Init();
	bool		ParseChannelList(std::string& htmlResponse);
	std::string GetStreamLink(int id, bool hd);
	std::string GetStreamLink(const std::string& str_id, bool hd);
   const CWeebTv::channel* const GetChannel(const std::string& str_id);

	virtual bool OnSettingChanging(const CSetting* setting) override;
	virtual void OnSettingChanged(const CSetting* setting) override;

	std::string m_username, m_password;
private:

	CCurlFile m_connection;
	ChannelMap m_channelList;
};

