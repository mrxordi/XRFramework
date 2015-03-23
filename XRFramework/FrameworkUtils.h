#pragma once

class CFUtil
{
public:
	static std::string GetNextFilename(const std::string &fn_template, int max);
	static std::string GetNextPathname(const std::string &path_template, int max);

};
