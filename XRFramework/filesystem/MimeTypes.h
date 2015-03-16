#pragma once
#include <map>
#include <string>

class CUrl;
class FileItem;

class Mime
{
public:
	static std::string GetMimeType(const std::string &extension);
	static std::string GetMimeType(const CUrl& url, bool lookup = true);
	static std::string GetMimeType(const FileItem& item);

private:
	static std::map<std::string, std::string> m_mimetypes;
};