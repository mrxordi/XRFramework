#pragma once
#include "Job.h"

#define PCRE_STATIC 1
#ifdef _DEBUG
#pragma comment(lib, "pcred.lib")
#else  // ! _DEBUG
#pragma comment(lib, "pcre.lib")
#endif // ! _DEBUG
#include <pcre.h>
typedef std::pair<int, int> GROUPENTRY;
typedef std::vector<GROUPENTRY> MATCH;

class CPcre;

class CPcreJob : public CJob
{
public:
	enum  MODE {
		NOSTUDY = 0,
		STUDY = 1,
		JITSTUDY
	};
	class JobContext {
	public:
		JobContext(std::string& dataStr, int vecSize,
			int dataOffset = 0, int maxNumberOfCharsTotest = -1) : vectorSize(vecSize),
			dataoffset(dataOffset), maxNumberOfCharsToTest(maxNumberOfCharsTotest), data(std::move(dataStr)) {}
		std::string data;
		std::vector<MATCH> matches;
		int vectorSize;
		int dataoffset;
		int maxNumberOfCharsToTest;
		void* extra;
	};
	CPcreJob(std::string pattern, MODE study = MODE::NOSTUDY, bool caseless = false, bool utf8 = false);
	virtual ~CPcreJob();
	virtual void InitValues(bool caseless = false, bool utf8 = false);
	virtual void Cleanup();
	virtual unsigned int Run(CPcre* callback);
	virtual bool DoWork() override;
	virtual int  FullMatch(std::string text, int vectorSize, std::vector<MATCH> &vector, int maxNumberOfCharsToTest = -1, int startOffset = 0);
	virtual int AddData(JobContext& data) { if(!m_bRunning) m_data.push_back(data); return m_data.size(); }
	virtual const char * GetType() const override;

	virtual bool operator ==(const CJob* job) const override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	static void LogBuildInfo();
	static bool IsUtf8Supported();
	static bool IsJitSupported();
	virtual bool CompileRegexp(std::string pattern, MODE study);

private:
	static int m_iJitSupported;
	static int m_iUtf8Supported;

	bool m_jitCompiled,
		m_bMatched,
		m_bUseUtf,
		m_bRunning;
	int m_iMatchCount, m_iOptions;

	std::vector<JobContext> m_data;
	std::string m_pattern;

	pcre *reCompiled;
	pcre_extra *pcreExtra;
	pcre_jit_stack *m_jitStack;

	CPcre* m_creatorCallback;
};

