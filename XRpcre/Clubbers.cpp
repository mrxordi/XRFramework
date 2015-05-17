#include "stdafx.h"
#include "Clubbers.h"
#include "XRThreads/SingleLock.h"
#include "filesystem/File.h"
#include <sstream>
#include <iostream>
#include <locale>

CClubbers::CClubbers()
{
	//m_page_pcre = new CPcreJob("(?<=\\<a href=\")(http:\/\/www.4clubbers.com.pl\/electro-electro-house\/[0-9]{7}[a-z0-9\-]*\.html)\" id=\"thread_title_[0-9]+\" style=\"\">([^<>]*)<\/a>+", CPcreJob::JITSTUDY);
	m_page_pcre = new CPcreJob("<a href=\"(http:\/\/www.4clubbers.com.pl\/electro-electro-house\/[0-9]{7}[a-z0-9\-]*\.html)\" id=\"thread_title_[0-9]+\" style=\"[a-z\-\:\;]*?\">(?:<span style=\".*?\">)*?([^<>]*)(?:<\/span>)*<\/a>+", CPcreJob::JITSTUDY);
	m_post_pcre = new CPcreJob("(?<=data-url=\')(http:\/\/[\/a-z0-9\.A-Z]+\.html)(?=\' data-link-type=\'popup\')", CPcreJob::JITSTUDY);
	m_zippy_pcre = new CPcreJob("(?<=\\<script type\=\"text\/javascript\"\>)[^<]*dlbutton[^<]+(?!<\/script>)", CPcreJob::JITSTUDY);
	std::string data;
	m_connection.SetReferer("www.4clubbers.com.pl/index.php");
	m_connection.SetUserAgent("Mozilla/5.0 (Windows; U; Windows NT 5.1; en-GB; rv:1.9.0.3) Gecko/2008092417 Firefox/3.0.3");
	m_connection.Post("http://www.4clubbers.com.pl/login.php?do=login", "&do=login&securitytoken=guest&vb_login_md5password=f86c81e453f731c6c05a184d828b8f6e&vb_login_md5password_utf=f86c81e453f731c6c05a184d828b8f6e&vb_login_password=f86c81e453f731c6c05a184d828b8f6e&vb_login_username=xordi", data);
	m_connection.Get("http://www.4clubbers.com.pl/index.php", data);
	m_connection.Get("http://www.4clubbers.com.pl/electro-electro-house/3", data);

	CPcreJob::JobContext context(data, 10);

	m_page_pcre->AddData(context);
	 int id = m_page_pcre->Run(this);
}


CClubbers::~CClubbers()
{
}

void CClubbers::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
	LOGDEBUG("Job Completed: %u with %s", jobID, success? "success": "failure");
	CPcreJob* pcrejob = reinterpret_cast<CPcreJob*>(job);
	if (m_page_pcre == pcrejob) {
		int id = m_post_pcre->Run(this);
	}
	if (m_post_pcre == pcrejob) {
		std::string data;
		int i = 0;
		for(auto& iter : m_zippyList)
		{
			m_connection.SetUserAgent("Mozilla/5.0 (Windows; U; Windows NT 5.1; en-GB; rv:1.9.0.3) Gecko/2008092417 Firefox/3.0.3");
			m_connection.SetReferer(iter.second);
 			m_connection.Get(iter.first, data);
//  			m_hddFile.OpenForWrite(StringUtils::Format("special://app/zippysong%i.html", i++).c_str(), true);
//  			m_hddFile.Write(data.c_str(), data.size());
//  			m_hddFile.Close();
 			CPcreJob::JobContext ctx(data, 10);
 			m_zippy_pcre->AddData(ctx);
		}
		m_zippy_pcre->Run(this);
	}
}

void CClubbers::OnJobProgress(unsigned int jobID, unsigned int progress, unsigned int total, const CJob *job)
{
	
}

void CClubbers::OnMatchFound(const CPcreJob::JobContext& context, unsigned int progress, const CPcreJob* job)
{
	XR::CSingleLock lock(m_callbackCritical);
	std::string data;

	if (job == m_page_pcre)
	{
		for (int i = 0; i < context.matches.size(); i++)
		{
			MATCH singlematch = context.matches[i];
			std::string result = context.data.substr(singlematch[1].first, singlematch[1].second - singlematch[1].first);
			std::string title = context.data.substr(singlematch[2].first, singlematch[2].second - singlematch[2].first);
			m_connection.Get(result, data);
			CPcreJob::JobContext ctx(data, 10);
			LOGINFO("----------------------------------------------------");
			LOGINFO("- Found post: %s", title.c_str());
			LOGINFO("- Link: %s", result.c_str());
			LOGINFO("----------------------------------------------------");

			std::string* str = new std::string(result);
			ctx.extra = (void*)str;
			m_post_pcre->AddData(ctx);
		}
		return;
	}
	if (job == m_post_pcre)
	{
		static int k = 0;
		for (int i = 0; i < context.matches.size(); i++)
		{
			MATCH singlematch = context.matches[i];
			std::string result = context.data.substr(singlematch[1].first, singlematch[1].second - singlematch[1].first);
			m_zippyList.push_back(std::make_pair<>(result, std::string(*(std::string*)context.extra)));
		}
		delete (std::string*)context.extra;
		return;
	}
	std::string singleScript = {""};
	if (job == m_zippy_pcre)
	{
		static int k = 0;
		if (progress > k + 1) {
			LOGERR("Match not found - skipping %i positions.", progress - k-1);
			k += progress - k-1;
		}

		for (int i = 0; i < context.matches.size(); i++)
		{
			MATCH singlematch = context.matches[i];
			singleScript += context.data.substr(singlematch[0].first, singlematch[0].second - singlematch[0].first);
			singleScript + "\n";
		}
		NormalizeScript(singleScript);
		CClubbers::ParserContext context;
		if (ParseScript(singleScript, m_zippyList[k], context)) {
			m_parsedContextList.push_back(context);
			m_zippyScriptList.push_back(std::move(singleScript));
		}
		else {
			LOGERR("Failed to parse data.");
		}
		k++;
	}
}

void CClubbers::NormalizeScript(std::string& str)
{
	size_t where;
	while ((where = str.find("  ")) != std::string::npos)
	{
		str.replace(where, 2, " ");
	}

	while ((where = str.find("\n ")) != std::string::npos)
	{
		str.replace(where, 2, "\n");
	}
}

bool CClubbers::ParseScript(std::string& str, std::pair<std::string, std::string> linkAndReference, CClubbers::ParserContext& ctx)
{
	CPcreJob regex("(?<=var )([a-z] = ?[0-9]+)");
	std::vector<MATCH> matches;
	ctx.scriptString = str;
	ctx.urlAndReference = linkAndReference;
	

	if (regex.FullMatch(str, 10, matches) < 2)
	ctx.tokenStr = ctx.scriptString.substr(matches[0][0].first, matches[0][0].second - matches[0][0].first);

	std::stringstream steam;
	int a, b, i;
	char z,y;

	for (i = 4; i < ctx.tokenStr.size(); i++)
	{
		if (isdigit(ctx.tokenStr[i]))
		{
			steam << ctx.tokenStr[i];
		}
	}
	steam << " ";
	steam >> ctx.a;	
	//\\------------------------------------------------------------------------------------------------------------/

	matches.clear();
	regex.CompileRegexp("omg = [0-9*-/%+]+", CPcreJob::NOSTUDY);
	if (regex.FullMatch(str, 10, matches) < 2)
	ctx.tokenStr = ctx.scriptString.substr(matches[0][0].first, matches[0][0].second - matches[0][0].first);

	for (i = 6; i < ctx.tokenStr.size(); i++)
	{
		if (isdigit(ctx.tokenStr[i]))
		{
			steam << ctx.tokenStr[i];
		}
		else
		{
			steam << " ";
			steam >> a;
			z = ctx.tokenStr[i];
		}
	}
// 	for (; i < ctx.tokenStr.size(); i++)
// 	{
// 		steam << ctx.tokenStr[i];
// 	}
	steam << " ";
	steam >> b;
	switch (z) {
	case '+':
		ctx.omg = a + b;
		break;
	case '-':
		ctx.omg = a - b;
		break;
	case '*':
		ctx.omg = a * b;
		break;
	case '/':
		ctx.omg = a / b;
		break;
	case '%':
		ctx.omg = a % b;
		break;
	}
	//\\------------------------------------------------------------------------------------------------------------/
	a = b = i = 0;
	z = y = '\0';
	matches.clear();
	regex.CompileRegexp("omg[) ]+[ \-\+\*\/\%]+[0-9\/\*\-\+\%\(\)]+", CPcreJob::NOSTUDY);
	if (regex.FullMatch(str, 10, matches) < 2)
		ctx.tokenStr = ctx.scriptString.substr(matches[0][0].first, matches[0][0].second - matches[0][0].first);

	bool digitsinside = false;
	for (int i = 3; i < ctx.tokenStr.size(); i++)
	{
		if (ctx.tokenStr[i] == '(')
			a = Nawias(ctx, i);
		if (ctx.tokenStr[i] == '+' || ctx.tokenStr[i] == '-' || ctx.tokenStr[i] == '*' || ctx.tokenStr[i] == '/' || ctx.tokenStr[i] == '%')
			z = ctx.tokenStr[i];
	}

	switch (z)
	{
	case '+':
		ctx.b = ctx.omg + a;
		break;
	case '-':
		ctx.b = ctx.omg - a;
		break;
	case '*':
		ctx.b = ctx.omg * a;
		break;
	case '/':
		ctx.b = ctx.omg / a;
		break;
	case '%':
		ctx.b = ctx.omg % a;
		break;
	}

	//\\------------------------------------------------------------------------------------------------------------/
	a = b = i = 0;
	z = y = '\0';
	matches.clear();
	regex.CompileRegexp("\\+\\([a-z][-+*/%]\\d+\\)\\+", CPcreJob::NOSTUDY);
	if (regex.FullMatch(str, 10, matches))
		ctx.tokenStr = ctx.scriptString.substr(matches[0][0].first, matches[0][0].second - matches[0][0].first);

	for (int i = 2; i < ctx.tokenStr.size()-2; i++)
	{
		if (isdigit(ctx.tokenStr[i])) {
			steam << ctx.tokenStr[i];
			if (!isdigit(ctx.tokenStr[i+1]))
			{
				steam << " ";
				steam >> b;
			}
		}

		if (ctx.tokenStr[i] == '+' || ctx.tokenStr[i] == '-' || ctx.tokenStr[i] == '*' || ctx.tokenStr[i] == '/' || ctx.tokenStr[i] == '%') {
			z = ctx.tokenStr[i];
			continue;
		}

		switch (ctx.tokenStr[i]) {
		case 'a':
		{
			if (!a)
				a = ctx.a;
			else
				b = ctx.a;
			break;
		}
		case 'b':
		{
			if (!a)
				a = ctx.b;
			else
				b = ctx.b;
			break;
		}
		case 'e':				
		{
			if (!a)
				a = ctx.e;
			else
				b = ctx.e;
			break;
		}
		case 'o':
		{
			i += 2;
			if (!a)
				a = ctx.omg;
			else
				b = ctx.omg;
			break;
		}
		}
	}

	switch (z)
	{
	case '+':
		ctx.middlenumber = a + b;
		break;
	case '-':
		ctx.middlenumber = a - b;
		break;
	case '*':
		ctx.middlenumber = a * b;
		break;
	case '/':
		ctx.middlenumber = a / b;
		break;
	case '%':
		ctx.middlenumber = a % b;
		break;
	}

	//\\------------------------------------------------------------------------------------------------------------/
	matches.clear();


	regex.CompileRegexp("(\/d\/[\/a-zA-Z0-9]+)(?:\\\"\\+\\([a-z][-+*/%]\\d+\\)\\+\\\")(\/[a-zA-Z%0-9!_\.-]+.(mp3|wav))", CPcreJob::NOSTUDY);
	if (regex.FullMatch(str, 10, matches) > 0)
		ctx.tokenStr = ctx.scriptString.substr(matches[0][0].first, matches[0][0].second - matches[0][0].first);
	else
	{
		LOGERR("Not found string that builds a link: %s ", ctx.urlAndReference.first.c_str());
		return false;
	}
	
	std::string url;
	url += "http://";
	url += CURL(linkAndReference.first).GetHostName();
	url += ctx.scriptString.substr(matches[0][1].first, matches[0][1].second - matches[0][1].first);
	url += StringUtils::Format("%i", ctx.middlenumber);
	url += ctx.scriptString.substr(matches[0][2].first, matches[0][2].second - matches[0][2].first);
	CFile::Copy(url, "special://app/" + CURL::Decode(ctx.scriptString.substr(matches[0][2].first, matches[0][2].second - matches[0][2].first)));



	return true;
}

int CClubbers::Nawias(ParserContext& ctx, int& i)
{
	int a=0, b=0;
	char z = ' ';
	std::stringstream stream;
	bool digitinside = false;
	for (; i < ctx.tokenStr.size(); i++)
	{
		char ch = ctx.tokenStr[i];
		if (isdigit(ch)) {
			stream << ctx.tokenStr[i];
			digitinside = true;
		}
		if (digitinside && !isdigit(ctx.tokenStr[i+1])) {
			if (!a) {
				stream << " ";
				stream >> a;
				digitinside = false;
			} 
			else {
				stream << " ";
				stream >> b;
				digitinside = false;
			}
		}
		if (ctx.tokenStr[i] == '+' || ctx.tokenStr[i] == '-' || ctx.tokenStr[i] == '*' || ctx.tokenStr[i] == '/' || ctx.tokenStr[i] == '%')
			z = ctx.tokenStr[i];
	}

	switch (z)
	{
	case '+':
		return a + b;
	case '-':
		return a - b;
	case '*':
		return a * b;
	case '/':
		return a / b;
	case '%':
		return a % b;
	}
	return -1;
}
