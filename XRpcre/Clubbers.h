#pragma once
#include "XRFramework/utils/Pcre.h"
#include "XRFramework/filesystem/CurlFile.h"
#include "XRFramework/filesystem/File.h"
#include "XRFramework/utils/PcreJob.h"
#include "XRCommon/utils/StringUtils.h"
#include "XRCommon/log/Log.h"
#include "XRThreads/CriticalSection.h"
#include "XRCommon/utils/URL.h"

class CClubbers : public CPcre
{
public:
	struct ParserContext {
		int a = 0, b = 0, e = 0, omg = 0, middlenumber = 0;
		std::string tokenStr, urlString, scriptString;
		std::pair<std::string, std::string> urlAndReference;
	};
	enum token { none = 0, A = 1, B, E, O, M, G, add, minus, multiple, modulo, dziel, equal, digit};
	CClubbers();
	virtual ~CClubbers();

	virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job) override;

	virtual void OnJobProgress(unsigned int jobID, unsigned int progress, unsigned int total, const CJob *job) override;

	virtual void OnMatchFound(const CPcreJob::JobContext& context, unsigned int progress, const CPcreJob* job) override;

private:
	virtual void NormalizeScript(std::string& str);
	virtual bool ParseScript(std::string& str, std::pair<std::string, std::string> linkAndReference, ParserContext& ctx);
	virtual int Nawias(ParserContext& ctx, int& i);

	CPcreJob* m_page_pcre,
			*m_post_pcre,
			*m_zippy_pcre;

	std::vector<std::pair<std::string, std::string> > m_zippyList;
	std::vector<std::string> m_zippyScriptList;
	std::vector<ParserContext> m_parsedContextList;

	CCurlFile m_connection;
	CFile	  m_hddFile;
	XR::CCriticalSection m_callbackCritical;
};

