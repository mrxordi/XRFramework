#include "stdafxf.h"
#include "IInputStream.h"


IInputStream::IInputStream(EStreamType streamType) 
{
	m_streamType = streamType;
}
IInputStream::~IInputStream()
{
}

bool IInputStream::Open(const char* strFileName, const std::string& content)
{
	CURL url(strFileName);

	m_url = url;
	// get rid of any protocol options which might have sneaked in here
	// but keep them in m_url.
	url.SetProtocolOptions("");
	m_strFileName = url.Get();

	m_content = content;
	return true;
}

bool IInputStream::Open(const CFileItem& fileItem)
{
	m_url = fileItem.GetURL();

	m_strFileName = m_url.Get();
	m_content = fileItem.GetMimeType();
	return true;
}

void IInputStream::Close()
{
	m_strFileName = "";
}
