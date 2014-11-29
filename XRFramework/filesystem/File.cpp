#include "stdafxf.h"
#include "File.h"
#include "Base.h"
#include "IFile.h"
#include "log/Log.h"
#include "utils/URIUtils.h"
#include "Util.h"
#include "URL.h"



File::File(){}

File::~File(){}

bool File::Open(const std::string& strFileName, const unsigned int flags)
{
	const CURL pathToUrl(strFileName);
	return Open(pathToUrl, flags);
}

IFile* File::FileFactory(const CURL& url) {


}
