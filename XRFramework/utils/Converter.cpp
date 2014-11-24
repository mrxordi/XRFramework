#include "stdafxf.h"
#include "Converter.h"
#include "ConverterFactory.h"


Converter::Converter(const std::string&  sourceCharset, const std::string&  targetCharset, unsigned int targetSingleCharMaxLen, unsigned int timeOfExist) :
m_sourceCharset(sourceCharset), m_targetCharset(targetCharset), m_targetSingleCharMaxLen(targetSingleCharMaxLen), m_timeOfExist(timeOfExist), m_iconv(NO_ICONV), CCriticalSection()
{
}


Converter::~Converter()
{
	CSingleLock lock(*this);
	if (m_iconv != NO_ICONV)
		iconv_close(m_iconv);
	lock.Leave(); // ensure unlocking before final destruction
}

iconv_t Converter::GetConverter(CSingleLock& converterLock) {
	// ensure that this unique instance is locked externally
	if (&converterLock.get_underlying() != this)
		return NO_ICONV;

	if (m_iconv == NO_ICONV)
	{

		m_iconv = iconv_open(m_targetCharset.c_str(), m_sourceCharset.c_str());

		if (m_iconv == NO_ICONV)
			LOGERR("iconv_open() for \"%s\" -> \"%s\" failed, errno = %d (%s)",
			m_sourceCharset.c_str(), m_targetCharset.c_str(), errno, strerror(errno));
	}
	else {
		if (m_timeOfExist)
			m_timeOfExist += m_timeOfExist;
	}

	return m_iconv;
}