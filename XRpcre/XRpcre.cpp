// XRpcre.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "XRCommon/utils/SpecialProtocol.h"
#include <iostream>
#include "XRFramework/filesystem/CurlFile.h"
#include "XRFramework/filesystem/File.h"
#include "XRFramework/utils/PcreJob.h"
#include "XRCommon/utils/StringUtils.h"
#include "XRCommon/log/Log.h"
#include "Clubbers.h"



int _tmain(int argc, _TCHAR* argv[])
{
	CSpecialProtocol::InitializePaths();
	g_LogPtr->Create();
	g_LogPtr->Init(std::string("log.log"));
	CSpecialProtocol::LogPaths();
	CPcreJob::LogBuildInfo();
 	g_LogPtr->SetExtraLogLevels(LOGCURL);


	/*CClubbers pcre;*/
	

// 	pcre *reCompiled;
// 	pcre_extra *pcreExtra;
// 	int pcreExecRet;
// 	int subStrVec[30];
// 	const char *pcreErrorStr;
// 	int pcreErrorOffset;
// 	char *aStrRegex;
// 	char **aLineToMatch;
// 	const char *psubStrMatchStr;
// 	int j;
// 	char *testStrings[] = { "This should match... hello",
// 		"This could match... hello!",
// 		"More than one hello.. hello",
// 		"No chance of a match...",
// 		NULL };
// 
// 
// 	aStrRegex = "(.*)(hello)+"; 
// 	LOGINFO("Regex to use: %s\n", aStrRegex);
// 
// 	// First, the regex string must be compiled.
// 	reCompiled = pcre_compile(aStrRegex, 0, &pcreErrorStr, &pcreErrorOffset, NULL);
// 
// 	/* OPTIONS (second argument) (||'ed together) can be:
// 	PCRE_ANCHORED       -- Like adding ^ at start of pattern.
// 	PCRE_CASELESS       -- Like m//i
// 	PCRE_DOLLAR_ENDONLY -- Make $ match end of string regardless of \n's
// 	No Perl equivalent.
// 	PCRE_DOTALL         -- Makes . match newlins too.  Like m//s
// 	PCRE_EXTENDED       -- Like m//x
// 	PCRE_EXTRA          --
// 	PCRE_MULTILINE      -- Like m//m
// 	PCRE_UNGREEDY       -- Set quantifiers to be ungreedy.  Individual quantifiers
// 	may be set to be greedy if they are followed by "?".
// 	PCRE_UTF8           -- Work with UTF8 strings.
// 	*/
// 
// 	// pcre_compile returns NULL on error, and sets pcreErrorOffset & pcreErrorStr
// 	if (reCompiled == NULL) {
// 		LOGERR("ERROR: Could not compile '%s': %s\n", aStrRegex, pcreErrorStr);
// 		exit(1);
// 	} /* end if */
// 
// 	// Optimize the regex
// 	pcreExtra = pcre_study(reCompiled, 0, &pcreErrorStr);
// 
// 	/* pcre_study() returns NULL for both errors and when it can not optimize the regex.  The last argument is how one checks for
// 	errors (it is NULL if everything works, and points to an error string otherwise. */
// 	if (pcreErrorStr != NULL) {
// 		LOGERR("ERROR: Could not study '%s': %s\n", aStrRegex, pcreErrorStr);
// 		exit(1);
// 	} /* end if */
// 
// 	for (aLineToMatch = testStrings; *aLineToMatch != NULL; aLineToMatch++) {
// 		LOGINFO("String: %s\n", *aLineToMatch);
// 		LOGINFO("        %s\n", "0123456789012345678901234567890123456789");
// 		LOGINFO("        %s\n", "0         1         2         3");
// 
// 		/* Try to find the regex in aLineToMatch, and report results. */
// 		pcreExecRet = pcre_exec(reCompiled,
// 			pcreExtra,
// 			*aLineToMatch,
// 			strlen(*aLineToMatch),  // length of string
// 			0,                      // Start looking at this point
// 			0,                      // OPTIONS
// 			subStrVec,
// 			30);                    // Length of subStrVec
// 
// 		/* pcre_exec OPTIONS (||'ed together) can be:
// 		PCRE_ANCHORED -- can be turned on at this time.
// 		PCRE_NOTBOL
// 		PCRE_NOTEOL
// 		PCRE_NOTEMPTY */
// 
// 		// Report what happened in the pcre_exec call..
// 		//printf("pcre_exec return: %d\n", pcreExecRet);
// 		if (pcreExecRet < 0) { // Something bad happened..
// 			switch (pcreExecRet) {
// 			case PCRE_ERROR_NOMATCH: LOGERR("String did not match the pattern\n");        break;
// 			case PCRE_ERROR_NULL: LOGERR("Something was null\n");                      break;
// 			case PCRE_ERROR_BADOPTION: LOGERR("A bad option was passed\n");                 break;
// 			case PCRE_ERROR_BADMAGIC: LOGERR("Magic number bad (compiled re corrupt?)\n"); break;
// 			case PCRE_ERROR_UNKNOWN_NODE: LOGERR("Something kooky in the compiled re\n");      break;
// 			case PCRE_ERROR_NOMEMORY: LOGERR("Ran out of memory\n");                       break;
// 			default: LOGERR("Unknown error\n");                           break;
// 			} /* end switch */
// 		}
// 		else {
// 			LOGINFO("Result: We have a match!\n");
// 
// 			// At this point, rc contains the number of substring matches found...
// 			if (pcreExecRet == 0) {
// 				LOGINFO("But too many substrings were found to fit in subStrVec!\n");
// 				// Set rc to the max number of substring matches possible.
// 				pcreExecRet = 30 / 3;
// 			} /* end if */
// 
// 			// Do it yourself way to get the first substring match (whole pattern):
// 			// char subStrMatchStr[1024];
// 			// int i, j
// 			// for(j=0,i=subStrVec[0];i<subStrVec[1];i++,j++) 
// 			//   subStrMatchStr[j] = (*aLineToMatch)[i];
// 			// subStrMatchStr[subStrVec[1]-subStrVec[0]] = 0;
// 			//printf("MATCHED SUBSTRING: '%s'\n", subStrMatchStr);
// 
// 			// PCRE contains a handy function to do the above for you:
// 			for (j = 0; j<pcreExecRet; j++) {
// 				pcre_get_substring(*aLineToMatch, subStrVec, pcreExecRet, j, &(psubStrMatchStr));
// 				LOGINFO("Match(%2d/%2d): (%2d,%2d): '%s'\n", j, pcreExecRet - 1, subStrVec[j * 2], subStrVec[j * 2 + 1], psubStrMatchStr);
// 			} /* end for */
// 
// 			// Free up the substring
// 			pcre_free_substring(psubStrMatchStr);
// 		}  /* end if/else */
// 		std::cout << "\n";
// 
// 	} /* end for */
// 
// 	// Free up the regular expression.
// 	pcre_free(reCompiled);
// 
// 	// Free up the EXTRA PCRE value (may be NULL at this point)
// 	if (pcreExtra != NULL)
// 		pcre_free(pcreExtra);
// 
// 	// We are all done..
int a;
 	std::cin >> a;
	CJobManager::GetInstance().CancelJobs();

	return 0;

} /* end func main */

void StaryRun()
{
	CCurlFile connection;
	CFile fileToSave;
	std::string data = "", postdata = "";
	// 	connection.SetCookie("4clubbers");
	// 	clubbers.SetFileName("login.php");
	// 	clubbers.SetOption("do", "login");
	// 	LOGINFO("Url wyglada teraz tak: %s", clubbers.Get().c_str());
	// 	connection.Close();
	connection.SetReferer("www.4clubbers.com.pl/index.php");
	connection.SetUserAgent("Mozilla/5.0 (Windows; U; Windows NT 5.1; en-GB; rv:1.9.0.3) Gecko/2008092417 Firefox/3.0.3");
	connection.Post("http://www.4clubbers.com.pl/login.php?do=login", "&do=login&securitytoken=guest&vb_login_md5password=f86c81e453f731c6c05a184d828b8f6e&vb_login_md5password_utf=f86c81e453f731c6c05a184d828b8f6e&vb_login_password=f86c81e453f731c6c05a184d828b8f6e&vb_login_username=xordi", data);
	fileToSave.OpenForWrite("special://app/login.html", true);
	fileToSave.Write(data.c_str(), data.size());
	fileToSave.Flush();
	fileToSave.Close();
	connection.Post("http://www.4clubbers.com.pl/index.php", "", data);
	fileToSave.OpenForWrite("special://app/index.html", true);
	fileToSave.Write(data.c_str(), data.size());
	fileToSave.Flush();
	fileToSave.Close();

	connection.Get("http://www.4clubbers.com.pl/electro-electro-house/", data);
	fileToSave.OpenForWrite("special://app/electro.html", true);
	fileToSave.Write(data.c_str(), data.size());
	fileToSave.Flush();
	fileToSave.Close();

	// 	int a;
	// 	std::cin >> a;

	CPcreJob pcre_job_katalog("(?<=\\<a href=\")(http:\/\/www.4clubbers.com.pl\/electro-electro-house\/[0-9]{7}[a-z0-9\-]*\.html)\" id=\"thread_title_[0-9]+\" style=\"\">([^<>]*)<\/a>+", CPcreJob::JITSTUDY);
	CPcreJob pcre_job_song("(?<=data-url=\')(http:\/\/[a-z0-9\.]*.*\.html)(?=\' data-link-type=\'popup\')");
	std::vector<MATCH> matchesFound;
	std::vector<MATCH> LinksFound;
	std::vector<std::string> LingsStrings;

	pcre_job_katalog.FullMatch(data, 10, matchesFound);

	for (int i = 0; i < matchesFound.size(); i++)
	{
		std::string urlpath = data.substr(matchesFound[i][1].first, matchesFound[i][1].second - matchesFound[i][1].first);
		CURL url(urlpath.c_str());
		LOGDEBUG("Opening single url: %s", url.Get().c_str());
		connection.Get(url.Get(), postdata);
		fileToSave.OpenForWrite(StringUtils::Format("special://app/song%i.html", i).c_str(), true);
		fileToSave.Write(postdata.c_str(), postdata.size());
		fileToSave.Flush();
		fileToSave.Close();
		LOGDEBUG("Post numer %i", i);
		pcre_job_song.FullMatch(postdata, 10, LinksFound);
		for (int j = 0; j < LinksFound.size(); j++) {
			std::string str = postdata.substr(LinksFound[j][1].first, LinksFound[j][1].second - LinksFound[j][1].first);
			LingsStrings.push_back(str);
			LOGDEBUG("Dodalem %s do listy linkow, pozycja: %i", LingsStrings[LingsStrings.size() - 1].c_str(), LingsStrings.size() - 1);
		}
		LinksFound.clear();
	}
}
