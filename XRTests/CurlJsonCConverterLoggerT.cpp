#include "stdafx.h"
#include <json/json.h>

#include "CurlJsonCConverterLoggerT.h"

#include "../XRFramework/filesystem/CurlFile.h"
#include "../XRFramework/filesystem/URL.h"
#include "../XRFramework/utils/StringUtils.h"
#include "../XRFramework/utils/StringConverter.h"
#include "../XRFramework/log/Log.h"


void CurlJsonCharsetConverterLoggerTests(void) {

	CCurlFile file;

	LOGINFO("You are %s internet.", file.IsInternet() ? "connected to" : "disconnected from");
	file.SetUserAgent("XBMC");

	std::string str;
	file.Post("http://www.weeb.tv/api/setPlayer", "channel=tvp22hd&platform=XBMC&username=xordi@wp.pl&password=8085", str);

	LOGINFO(str.c_str());
	str = CURL::Decode(str);
	LOGINFO(str.c_str());

	file.Post("http://www.weeb.tv/api/getChannelList", "username=xordi@wp.pl&password=8085", str);
	LOGINFO(str.c_str());


	Json::Value root;
	Json::Reader reader;

	reader.parse(str, root, false);

	std::string infow1250("Root zawiera %i wpis�w");
	std::string infoutf;

	StringConverter::Win1250ToUtf8(infow1250, infoutf);


	LOGINFO("Root zawiera %i wpis�w", root.size());
	LOGINFO(infoutf.c_str(), root.size());

	std::wstring wStrConv;
	std::string	utfString;
	int i;
	Json::ValueIterator it = root.begin();
	for (i = 0; i < root.size(); ++i, it++) {
		std::string buff = (*it)["channel_title"].asString();

		LOGINFO("%i - %s", i + 1, (*it)["channel_title"].asString().c_str());
		utfString += StringUtils::Format("%i - %s\n", i + 1, (*it)["channel_title"].asString().c_str());
	}


	StringConverter::utf8ToW(utfString, wStrConv, false);
}