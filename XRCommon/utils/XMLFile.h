#pragma once
#include <tinyxml2.h>
#include "filesystem/File.h"

class CXMLFile : public tinyxml2::XMLDocument
{
public:
	CXMLFile() : tinyxml2::XMLDocument() {};

	virtual ~CXMLFile();

	bool LoadFile(const char* _filename);
	bool LoadFile(const std::string& _filename);
	bool SaveFile(const char* _filename) const;
	bool SaveFile(const std::string& filename) const;

	bool Parse(const std::string& data);
};

