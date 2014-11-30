#include "stdafxf.h"
#include "ConverterFactory.h"
#include "log/Log.h"
#include "../XRThreads/SingleLock.h"
#include "../XRThreads/SystemClock.h"

ConverterFactory::VectorConverter ConverterFactory::m_vConverters;
CCriticalSection ConverterFactory::m_critSect;

Converter* ConverterFactory::GetConverter(const std::string&  sourceCharset, const std::string&  targetCharset) {
	CSingleLock lock(m_critSect);

	if (sourceCharset.empty() || sourceCharset.empty()) {
		LOGERR("One of the charsets missing!");
		return nullptr;
	}
	std::string ident = sourceCharset + targetCharset;

	for (unsigned int i = 0; i < m_vConverters.size(); ++i)
	{
		ConverterUnit& converterunit = m_vConverters[i];
		if (converterunit.m_ident == ident) {
			converterunit.m_unloadDelayStartTick = XR::SystemClockMillis();
			return converterunit.m_converter;
		}
	}

	//Converter not present, sooo.... Let's create one for us;
	LOGDEBUG("Converter (%s --> %s) not exist, creating new.", sourceCharset.c_str(), targetCharset.c_str());
	Converter* pNewConverter = nullptr;
	lock.Leave();
	if (targetCharset == "UTF-8")
		pNewConverter = CreateConverter(sourceCharset, targetCharset, 4);
	else
		pNewConverter = CreateConverter(sourceCharset, targetCharset);

	if (nullptr == pNewConverter)
		LOGFATAL("Cannot create new converter! (not enough of memory?)");
	lock.Enter();
	return pNewConverter;
}

Converter* ConverterFactory::CreateConverter(const std::string&  sourceCharset, const std::string&  targetCharset, unsigned int targetSingleCharMaxLen, unsigned int timeOfExist) {
	CSingleLock lock(m_critSect);

	if (targetCharset == "UTF-8" && targetSingleCharMaxLen != 4) {
		LOGFATAL("Failed to create converter (%s --> %s) as single character max Lenght for UTF-8 is 4.", sourceCharset.c_str(), targetCharset.c_str());
		return nullptr;
	}

	std::string ident = sourceCharset + targetCharset;

	for (unsigned int i = 0; i < m_vConverters.size(); ++i)			//check for duplicate
	{
		ConverterUnit& converterunit = m_vConverters[i];
		if (converterunit.m_ident == ident) {
			converterunit.m_unloadDelayStartTick = XR::SystemClockMillis();
			return converterunit.m_converter;
		}
	}

	Converter* pNewConverter = nullptr;


	pNewConverter = new Converter(sourceCharset, targetCharset, targetSingleCharMaxLen, timeOfExist);
	CSingleLock convLock(*pNewConverter);
	if (NO_ICONV == pNewConverter->GetConverter(convLock)) {
		convLock.Leave();
		delete pNewConverter;
		return nullptr;
	}

	ConverterFactory::ConverterUnit newUnit;
	newUnit.m_converter = pNewConverter;
	newUnit.m_ident = ident;
	newUnit.m_unloadDelayStartTick = XR::SystemClockMillis();

	m_vConverters.push_back(newUnit);
	//LOGDEBUG("Succesfully created converter (%s --> %s).", sourceCharset.c_str(), targetCharset.c_str());

	return pNewConverter;
}

void ConverterFactory::DestroyConverter(const std::string&  sourceCharset, const std::string&  targetCharset) {
	std::string ident = sourceCharset + targetCharset;

	for (unsigned int i = 0; i < m_vConverters.size(); ++i)
	{
		ConverterUnit& converterunit = m_vConverters[i];
		if (converterunit.m_ident == ident) {
			LOGDEBUG("Destroyed (%s --> %s) converter.", converterunit.m_converter->GetSourceCharset().c_str(),
				converterunit.m_converter->GetTargetCharset().c_str());
			delete converterunit.m_converter;
		}
		m_vConverters.erase(m_vConverters.begin() + i);
	}
}


void ConverterFactory::CheckIdleDestroy() {
	CSingleLock lock(m_critSect);

	for (unsigned int i = 0; i < m_vConverters.size(); ++i)
	{
		ConverterUnit& converterunit = m_vConverters[i];
		if (converterunit.m_converter->GetTimeOfExist() == 0) {
			delete converterunit.m_converter;
		}
		else {
			unsigned int tickcount = XR::SystemClockMillis() - converterunit.m_unloadDelayStartTick;
			if (tickcount > converterunit.m_converter->GetTimeOfExist())
				delete converterunit.m_converter;

		}
		LOGDEBUG("Deleted (%s --> %s) converter as Idled(%ums)", converterunit.m_converter->GetSourceCharset().c_str(),
			converterunit.m_converter->GetTargetCharset().c_str(), converterunit.m_converter->GetTimeOfExist());

		m_vConverters.erase(m_vConverters.begin() + i);
	}
}

void ConverterFactory::DestroyAll() {
	CSingleLock lock(m_critSect);
	LOGDEBUG("Deleting %i converters.", m_vConverters.size());

	unsigned int i = 0;

	for (i = 0; i < m_vConverters.size(); i++)
	{
		ConverterUnit& converterunit = m_vConverters[i];
		delete converterunit.m_converter;
	}
	m_vConverters.clear();
}
