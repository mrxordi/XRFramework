#include "stdafx.h"
#include <json/json.h>

#include "CurlJsonCConverterLoggerT.h"

#include "../XRFramework/filesystem/CurlFile.h"
#include "../XRFramework/filesystem/URL.h"
#include "../XRFramework/filesystem/SpecialProtocol.h"
#include "../XRFramework/filesystem/FileItem.h"
#include "../XRFramework/filesystem/Directory.h"
#include "../XRFramework/utils/StringUtils.h"
#include "../XRFramework/utils/StringConverter.h"
#include "../XRFramework/log/Log.h"
#include "../XRFramework/filesystem/File.h"
#include "../XRFramework/filesystem/FileItem.h"
#include "../XRFramework/filesystem/FileOperationJob.h"
#include "../XRFramework/utils/JobManager.h"
#include "Application.h"


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

	std::string infow1250("Root zawiera %i wpisów");
	std::string infoutf;

	StringConverter::Win1250ToUtf8(infow1250, infoutf);


	LOGINFO("Root zawiera %i wpisów", root.size());
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


	LOGINFO("*----------------------------------------------------------------------------*");
	CURL filenameurl("special://app/test.txt");
	CURL filenameurlcopy("special://app/testCOPY.txt");
	CURL filenameurlRENAMED("special://app/testCOPYRENAMED.txt");

	File winfile;
	LOGDEBUG("File test to open %s.", winfile.Exists(filenameurl) ? "exist" : "doesn't exist");
	LOGDEBUG("Opening file for write - %s.", winfile.OpenForWrite(filenameurl) ? "succes" : "failed");
	LOGDEBUG("File has %lld bytes.", winfile.GetLength());
	winfile.Seek(0, FILE_END);
	const char* texttowrite = { "Text addition. \n" }; 
	LOGDEBUG("File has %i bytes.", winfile.Write(texttowrite, strlen(texttowrite)*sizeof(char)));
	LOGDEBUG("File has %lld bytes.", winfile.GetLength());
	LOGDEBUG("File closing.");
	winfile.Close();
	File::Copy("http://i.wp.pl/a/f/jpeg/34136/afp_ukraina_obwe_600.jpeg", "special://app/cache/afp_ukraina_obwe_600.jpeg", g_applicationPtr, 0);
	File::Copy("http://i.wp.pl/a/f/jpeg/34136/afp_ukraina_obwe_600.jpeg", "special://app/cache/skopiowany_obrazek.jpeg", g_applicationPtr, 0);

	LOGINFO("*----------------------------------------------------------------------------*");
	CURL httpurl("http://i.wp.pl/a/f/jpeg/34136/lanisek_scr2_lanisek_screen_nr1_650_360.jpeg");

	//File winfile;
	LOGDEBUG("Opening http url - %s.", winfile.Open(httpurl) ? "succes" : "failed");
	LOGDEBUG("File test CURLFILE to open %s.", winfile.Exists(httpurl) ? "exist" : "doesn't exist");
	LOGDEBUG("Mime type of url - %s.", winfile.GetContentMimeType().c_str());
	LOGDEBUG("File has %lld bytes.", winfile.GetLength());
	LOGDEBUG("File closing.");
	winfile.Close();
	LOGINFO("*----------------------------------------------------------------------------*");

	FileItemPtr fileitemcache(new FileItem("special://app/cache/", true));
	LOGDEBUG("Checking for folder(%s) exist - %s.", fileitemcache->GetPath().c_str(), fileitemcache->Exists() ? "succes" : "failed");

	FileItemPtr fileitemcache2(new FileItem("special://app/cache2/", true));

	if (!fileitemcache2->Exists()) {
		LOGDEBUG("Removing folder(%s) - %s.", fileitemcache2->GetPath().c_str(), Directory::Remove(fileitemcache2->GetPath()) ? "succes" : "failed");
	}

	LOGDEBUG("Creating folder(%s) - %s.", fileitemcache2->GetPath().c_str(), Directory::Create(fileitemcache2->GetPath()) ? "succes" : "failed");



	FileItemList itemlist;
	Directory::Hints hints;
	hints.mask = "";
	hints.flags = DIR_FLAG_NO_FILE_DIRS;

	if (Directory::GetDirectory("special://app/", itemlist, hints, true)) {
		LOGDEBUG("In folder(%s) found %i files.", itemlist.GetPath().c_str(), itemlist.Size());

		for(int i = 0; i < itemlist.Size(); ++i) {
			LOGDEBUG("In folder(%s) - %i.%s. Mimetype: %s", fileitemcache2->GetPath().c_str(), i, URIUtils::GetFileName(itemlist[i]->GetPath()).c_str(), itemlist[i]->GetMimeType().c_str());
		}
	}
	LOGINFO("*----------------------------------------------------------------------------*");
	//FileItemList itemlist2;
	//Directory::Hints hints2;
	//itemlist2.Add(FileItemPtr(new FileItem("http://www78.zippyshare.com/d/60151792/14326/Tiesto-Club-Life-Podcast-382-27-07-2014-www.HousePlanet.DJ.mp3", false)));
	//FileOperationJob* operation = new FileOperationJob(FileOperationJob::ActionCopy, itemlist2, "special://app/cache5", true);
	//operation.DoWork();
	//CJobManager::GetInstance().AddJob(operation, operation, CJob::PRIORITY_HIGH);
	//CJobManager::GetInstance().;
	File::Copy("http://www78.zippyshare.com/d/60151792/14326/Tiesto-Club-Life-Podcast-382-27-07-2014-www.HousePlanet.DJ.mp3", "special://app/cache/Tiesto-Club-Life-Podcast-382-27-07-2014-www.HousePlanet.DJ.mp3");


}