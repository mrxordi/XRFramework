#pragma once
#include <iconv.h>
#include "../XRThreads/SingleLock.h"
#include "../XRThreads/CriticalSection.h"

#define CONVERTER_ALIVE_TIME 60*1000  //30 sec

#define NO_ICONV ((iconv_t)-1)

class Converter : public CCriticalSection
{
public:
	Converter(const std::string&  sourceCharset, const std::string&  targetCharset, unsigned int targetSingleCharMaxLen = 1, unsigned int timeOfExist = CONVERTER_ALIVE_TIME);
	Converter(const Converter& other);
	~Converter();

	iconv_t GetConverter(CSingleLock& converterLock);
	unsigned int GetTimeOfExist() { return m_timeOfExist; }

	std::string GetSourceCharset(void) const  { return m_sourceCharset; }
	std::string GetTargetCharset(void) const  { return m_targetCharset; }
	unsigned int GetTargetSingleCharMaxLen(void) const  { return m_targetSingleCharMaxLen; }

private:
	std::string         m_sourceCharset;
	std::string         m_targetCharset;
	iconv_t             m_iconv;
	unsigned int        m_targetSingleCharMaxLen;
	unsigned int		m_timeOfExist;

};

