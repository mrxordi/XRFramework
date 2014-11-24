#pragma once
#include <vector>
#include "Converter.h"
#include "../XRThreads/CriticalSection.h"

class ConverterFactory
{
public:
	class ConverterUnit {
	public:
		Converter* m_converter;
		unsigned int m_unloadDelayStartTick;
		std::string m_ident;
	};
	typedef std::vector <ConverterFactory::ConverterUnit> VectorConverter;

	static Converter* GetConverter(const std::string&  sourceCharset, const std::string&  targetCharset);
	static Converter* CreateConverter(const std::string&  sourceCharset, const std::string&  targetCharset, unsigned int targetSingleCharMaxLen = 1, unsigned int timeOfExist = CONVERTER_ALIVE_TIME);
	static void DestroyConverter(const std::string&  sourceCharset, const std::string&  targetCharset);

	static void CheckIdleDestroy();
	static void DestroyAll();

private:
	static VectorConverter m_vConverters;
	static CCriticalSection m_critSect;
};

