#pragma once
#include <map>
#include <string>

class CURL;
class FileItem;

class Mime
{
public:
	static std::string GetMimeType(const std::string &extension);
	static std::string GetMimeType(const CURL& url, bool lookup = true);
	static std::string GetMimeType(const FileItem& item);

private:
	static std::map<std::string, std::string> m_mimetypes;
};