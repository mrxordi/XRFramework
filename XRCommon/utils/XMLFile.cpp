#include "stdafx.h"
#include "XMLFile.h"
#include "XRFramework/utils/AutoBuffer.h"

CXMLFile::~CXMLFile()
{
}

bool CXMLFile::LoadFile(const char* _filename)
{
	return LoadFile(std::string(_filename));
}

bool CXMLFile::LoadFile(const std::string& _filename)
{
	CFile file;
	auto_buffer buffer;

	if (file.LoadFile(_filename, buffer) <= 0) {
		SetError(tinyxml2::XMLError::XML_ERROR_FILE_READ_ERROR, nullptr, nullptr);
		return false;
	}
	Clear();

	std::string data(static_cast<char*>(buffer.get()), buffer.length());
	buffer.clear();

	Parse(data);

	if (Error())
		return false;
	return true;	
}

bool CXMLFile::SaveFile(const char* _filename) const
{
	return SaveFile(std::string(_filename));
}

bool CXMLFile::SaveFile(const std::string& filename) const
{
	CFile file;
	if (file.OpenForWrite(filename, true))
	{
		tinyxml2::XMLPrinter printer;
		Accept(&printer);
		return file.Write(printer.CStr(), printer.CStrSize()) == printer.CStrSize();
	}
	return false;
}

bool CXMLFile::Parse(const std::string& data)
{
	return tinyxml2::XMLDocument::Parse(data.c_str(), data.size()) == 0;
}
