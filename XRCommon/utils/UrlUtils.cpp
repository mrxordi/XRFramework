#include "stdafx.h"
#include "UrlUtils.h"
#include "URL.h"
#include "SpecialProtocol.h"
#include "StringUtils.h"
#include <vector>

const char* UrlUtils::pictureExtensions = { ".png|.jpg|.jpeg|.bmp|.gif|.ico|.tif|.tiff|.tga|.pcx|.cbz|.zip|.cbr|.rar|.dng|.nef|.cr2|.crw|.orf|.arw|.erf|.3fr|.dcr|.x3f|.mef|.raf|.mrw|.pef|.sr2|.rss" };
const char* UrlUtils::musicExtensions = { ".nsv|.m4a|.flac|.aac|.strm|.pls|.rm|.rma|.mpa|.wav|.wma|.ogg|.mp3|.mp2|.m3u|.mod|.amf|.669|.dmf|.dsm|.far|.gdm|.imf|.it|.m15|.med|.okt|.s3m|.stm|.sfx|.ult|.uni|.xm|.sid|.ac3|.dts|.cue|.aif|.aiff|.wpl|.ape|.mac|.mpc|.mp+|.mpp|.shn|.zip|.rar|.wv|.nsf|.spc|.gym|.adx|.dsp|.adp|.ymf|.ast|.afc|.hps|.xsp|.xwav|.waa|.wvs|.wam|.gcm|.idsp|.mpdsp|.mss|.spt|.rsd|.mid|.kar|.sap|.cmc|.cmr|.dmc|.mpt|.mpd|.rmt|.tmc|.tm8|.tm2|.oga|.url|.pxml|.tta|.rss|.cm3|.cms|.dlt|.brstm|.wtv|.mka|.tak|.opus|.dff|.dsf|.sidstream|.oggstream|.nsfstream|.asapstream|.cdda" };
const char* UrlUtils::videoExtensions = { ".m4v|.3g2|.3gp|.nsv|.tp|.ts|.ty|.strm|.pls|.rm|.rmvb|.m3u|.m3u8|.ifo|.mov|.qt|.divx|.xvid|.bivx|.vob|.nrg|.img|.iso|.pva|.wmv|.asf|.asx|.ogm|.m2v|.avi|.bin|.dat|.mpg|.mpeg|.mp4|.mkv|.avc|.vp3|.svq3|.nuv|.viv|.dv|.fli|.flv|.rar|.001|.wpl|.zip|.vdr|.dvr-ms|.xsp|.mts|.m2t|.m2ts|.evo|.ogv|.sdp|.avs|.rec|.url|.pxml|.vc1|.h264|.rcv|.rss|.mpls|.webm|.bdmv|.wtv|.pvr|.disc" };
const char* UrlUtils::subtitlesExtensions = { ".utf|.utf8|.utf-8|.sub|.srt|.smi|.rt|.txt|.ssa|.text|.ssa|.aqt|.jss|.ass|.idx|.ifo|.rar|.zip" };

UrlUtils::UrlUtils(void)
{
}

UrlUtils::~UrlUtils(void)
{
}

const std::string UrlUtils::GetFileName(const std::string& strFileNameAndPath)
{
	if (IsURL(strFileNameAndPath))
	{
		CUrl url(strFileNameAndPath);
		return GetFileName(url.GetFileName());
	}

	/* find the last slash */
	const size_t slash = strFileNameAndPath.find_last_of("/\\");
	return strFileNameAndPath.substr(slash + 1);
}

const std::string UrlUtils::GetFileName(const CUrl& url)
{
	return GetFileName(url.GetFileName());
}

void UrlUtils::AddSlashAtEnd(std::string& strFolder)
{
	if (IsURL(strFolder))
	{
		CUrl url(strFolder);
		std::string file = url.GetFileName();
		if (!file.empty() && file != strFolder)
		{
			AddSlashAtEnd(file);
			url.SetFileName(file);
			strFolder = url.Get();
		}
		return;
	}

	if (!HasSlashAtEnd(strFolder))
	{
		if (IsDOSPath(strFolder))
			strFolder += '\\';
		else
			strFolder += '/';
	}
}

bool UrlUtils::HasSlashAtEnd(const std::string& strFile, bool checkURL /* = false */)
{
	if (strFile.empty()) return false;
	if (checkURL && IsURL(strFile))
	{
		CUrl url(strFile);
		std::string file = url.GetFileName();
		return file.empty() || HasSlashAtEnd(file, false);
	}
	char kar = strFile.c_str()[strFile.size() - 1];

	if (kar == '/' || kar == '\\')
		return true;

	return false;
}

void UrlUtils::RemoveSlashAtEnd(std::string& strFolder)
{
	if (IsURL(strFolder))
	{
		CUrl url(strFolder);
		std::string file = url.GetFileName();
		if (!file.empty() && file != strFolder)
		{
			RemoveSlashAtEnd(file);
			url.SetFileName(file);
			strFolder = url.Get();
			return;
		}
		if (url.GetHostName().empty())
			return;
	}

	while (HasSlashAtEnd(strFolder))
		strFolder.erase(strFolder.size() - 1, 1);
}

bool UrlUtils::IsURL(const std::string& strFile)
{
	return strFile.find("://") != std::string::npos;
}

bool UrlUtils::IsDOSPath(const std::string& path)
{
	if (path.size() > 1 && path[1] == ':' && isalpha(path[0]))
		return true;

	// windows network drives
	if (path.size() > 1 && path[0] == '\\' && path[1] == '\\')
		return true;

	return false;
}

bool UrlUtils::IsSpecial(const std::string& strFile)
{
	std::string strFile2(strFile);

	return IsProtocol(strFile2, "special");
}

bool UrlUtils::IsProtocol(const std::string& url, const std::string &type)
{
	return StringUtils::StartsWithNoCase(url, type + "://");
}

std::string UrlUtils::AddFileToFolder(const std::string& strFolder,
	const std::string& strFile)
{
	if (IsURL(strFolder))
	{
		CUrl url(strFolder);
		if (url.GetFileName() != strFolder)
		{
			url.SetFileName(AddFileToFolder(url.GetFileName(), strFile));
			return url.Get();
		}
	}

std::string strResult = strFolder;
	if (!strResult.empty())
		AddSlashAtEnd(strResult);

	// Remove any slash at the start of the file
	if (strFile.size() && (strFile[0] == '/' || strFile[0] == '\\'))
		strResult += strFile.substr(1);
	else
		strResult += strFile;

	// correct any slash directions
	if (!IsDOSPath(strFolder))
		StringUtils::Replace(strResult, '\\', '/');
	else
		StringUtils::Replace(strResult, '/', '\\');

	return strResult;
}

bool UrlUtils::HasParentInHostname(const CUrl& url)
{
	return url.IsProtocol("zip")
		|| url.IsProtocol("rar")
		|| url.IsProtocol("apk")
		|| url.IsProtocol("bluray")
		|| url.IsProtocol("udf");
}

bool UrlUtils::HasEncodedHostname(const CUrl& url)
{
	return HasParentInHostname(url)
		|| url.IsProtocol("musicsearch")
		|| url.IsProtocol("image");
}

bool UrlUtils::IsHD(const std::string& strFileName)
{
	CUrl url(strFileName);

	if (IsProtocol(strFileName, "special"))
		return IsHD(CSpecialProtocol::TranslatePath(strFileName));

	if (HasParentInHostname(url))
		return IsHD(url.GetHostName());

	return url.GetProtocol().empty() || url.IsProtocol("file");
}

bool UrlUtils::HasEncodedFilename(const CUrl& url)
{
	const std::string prot2 = url.GetTranslatedProtocol();

	// For now assume only (quasi) http internet streams use URL encoding
	return CUrl::IsProtocolEqual(prot2, "http") ||
		CUrl::IsProtocolEqual(prot2, "https");
}

bool UrlUtils::IsInternetStream(const std::string &path, bool bStrictCheck /* = false */)
{
	const CUrl pathToUrl(path);
	return IsInternetStream(pathToUrl, bStrictCheck);
}

bool UrlUtils::IsInternetStream(const CUrl& url, bool bStrictCheck /* = false */)
{
	if (url.GetProtocol().empty())
		return false;

	// there's nothing to stop internet streams from being stacked
	if (url.IsProtocol("stack"))
		return false;

	// Special case these
	if (url.IsProtocol("ftp") || url.IsProtocol("ftps") ||
		url.IsProtocol("dav") || url.IsProtocol("davs") ||
		url.IsProtocol("sftp"))
		return bStrictCheck;

	std::string protocol = url.GetTranslatedProtocol();
	if (CUrl::IsProtocolEqual(protocol, "http") || CUrl::IsProtocolEqual(protocol, "https") ||
		CUrl::IsProtocolEqual(protocol, "tcp") || CUrl::IsProtocolEqual(protocol, "udp") ||
		CUrl::IsProtocolEqual(protocol, "rtp") || CUrl::IsProtocolEqual(protocol, "sdp") ||
		CUrl::IsProtocolEqual(protocol, "mms") || CUrl::IsProtocolEqual(protocol, "mmst") ||
		CUrl::IsProtocolEqual(protocol, "mmsh") || CUrl::IsProtocolEqual(protocol, "rtsp") ||
		CUrl::IsProtocolEqual(protocol, "rtmp") || CUrl::IsProtocolEqual(protocol, "rtmpt") ||
		CUrl::IsProtocolEqual(protocol, "rtmpe") || CUrl::IsProtocolEqual(protocol, "rtmpte") ||
		CUrl::IsProtocolEqual(protocol, "rtmps"))
		return true;

	return false;
}

/* returns filename extension including period of filename */
std::string UrlUtils::GetExtension(const CUrl& url)
{
	return UrlUtils::GetExtension(url.GetFileName());
}

std::string UrlUtils::GetExtension(const std::string& strFileName)
{
	if (IsURL(strFileName))
	{
		CUrl url(strFileName);
		return GetExtension(url.GetFileName());
	}

	size_t period = strFileName.find_last_of("./\\");
	if (period == std::string::npos || strFileName[period] != '.')
		return std::string();

	return strFileName.substr(period);
}

bool UrlUtils::HasExtension(const std::string& strFileName)
{
	if (IsURL(strFileName))
	{
		CUrl url(strFileName);
		return HasExtension(url.GetFileName());
	}

	size_t iPeriod = strFileName.find_last_of("./\\");
	return iPeriod != std::string::npos && strFileName[iPeriod] == '.';
}

bool UrlUtils::HasExtension(const CUrl& url, const std::string& strExtensions)
{
	return HasExtension(url.GetFileName(), strExtensions);
}

bool UrlUtils::HasExtension(const std::string& strFileName, const std::string& strExtensions)
{
	if (IsURL(strFileName))
	{
		CUrl url(strFileName);
		return HasExtension(url.GetFileName(), strExtensions);
	}

	// Search backwards so that '.' can be used as a search terminator.
	std::string::const_reverse_iterator itExtensions = strExtensions.rbegin();
	while (itExtensions != strExtensions.rend())
	{
		// Iterate backwards over strFileName untill we hit a '.' or a mismatch
		for (std::string::const_reverse_iterator itFileName = strFileName.rbegin();
			itFileName != strFileName.rend() && itExtensions != strExtensions.rend() &&
			tolower(*itFileName) == *itExtensions;
		++itFileName, ++itExtensions)
		{
			if (*itExtensions == '.')
				return true; // Match
		}

		// No match. Look for more extensions to try.
		while (itExtensions != strExtensions.rend() && *itExtensions != '|')
			++itExtensions;

		while (itExtensions != strExtensions.rend() && *itExtensions == '|')
			++itExtensions;
	}

	return false;
}

void UrlUtils::RemoveExtension(std::string& strFileName)
{
	if (IsURL(strFileName))
	{
		CUrl url(strFileName);
		strFileName = url.GetFileName();
		RemoveExtension(strFileName);
		url.SetFileName(strFileName);
		strFileName = url.Get();
		return;
	}

	size_t period = strFileName.find_last_of("./\\");
	if (period != std::string::npos && strFileName[period] == '.')
	{
		std::string strExtension = strFileName.substr(period);
		StringUtils::ToLower(strExtension);
		strExtension += "|";

		std::string strFileMask;
		strFileMask = pictureExtensions;
		strFileMask += "|";
		strFileMask += musicExtensions;
		strFileMask += "|";
		strFileMask += videoExtensions;
		strFileMask += "|";
		strFileMask += subtitlesExtensions;
#if defined(TARGET_DARWIN)
		strFileMask += "|.py|.xml|.milk|.xpr|.xbt|.cdg|.app|.applescript|.workflow";
#else
		strFileMask += "|.py|.xml|.milk|.xpr|.xbt|.cdg";
#endif
		strFileMask += "|";

		if (strFileMask.find(strExtension) != std::string::npos)
			strFileName.erase(period);
	}
}

std::string UrlUtils::ReplaceExtension(const std::string& strFile,
	const std::string& strNewExtension)
{
	using namespace std;
	if (IsURL(strFile))
	{
		CUrl url(strFile);
		url.SetFileName(ReplaceExtension(url.GetFileName(), strNewExtension));
		return url.Get();
	}

	string strChangedFile;
	string strExtension = GetExtension(strFile);
	if (strExtension.size())
	{
		strChangedFile = strFile.substr(0, strFile.size() - strExtension.size());
		strChangedFile += strNewExtension;
	}
	else
	{
		strChangedFile = strFile;
		strChangedFile += strNewExtension;
	}
	return strChangedFile;
}

std::string UrlUtils::FixSlashesAndDups(const std::string& path, const char slashCharacter /* = '/' */, const size_t startFrom /*= 0*/)
{
	const size_t len = path.length();
	if (startFrom >= len)
		return path;

	std::string result(path, 0, startFrom);
	result.reserve(len);

	const char* const str = path.c_str();
	size_t pos = startFrom;
	do
	{
		if (str[pos] == '\\' || str[pos] == '/')
		{
			result.push_back(slashCharacter);  // append one slash
			pos++;
			// skip any following slashes
			while (str[pos] == '\\' || str[pos] == '/') // str is null-terminated, no need to check for buffer overrun
				pos++;
		}
		else
			result.push_back(str[pos++]);   // append current char and advance pos to next char

	} while (pos < len);

	return result;
}

std::string UrlUtils::CanonicalizePath(const std::string& path, const char slashCharacter /*= '\\'*/)
{
	using namespace std;
	ASSERT(slashCharacter == '\\' || slashCharacter == '/');

	if (path.empty())
		return path;

	const std::string slashStr(1, slashCharacter);
	vector<std::string> pathVec, resultVec;
	StringUtils::Tokenize(path, pathVec, slashStr);

	for (vector<std::string>::const_iterator it = pathVec.begin(); it != pathVec.end(); ++it)
	{
		if (*it == ".")
		{ /* skip - do nothing */
		}
		else if (*it == ".." && !resultVec.empty() && resultVec.back() != "..")
			resultVec.pop_back();
		else
			resultVec.push_back(*it);
	}

	std::string result;
	if (path[0] == slashCharacter)
		result.push_back(slashCharacter); // add slash at the begin

	result += StringUtils::Join(resultVec, slashStr);

	if (path[path.length() - 1] == slashCharacter  && !result.empty() && result[result.length() - 1] != slashCharacter)
		result.push_back(slashCharacter); // add slash at the end if result isn't empty and result isn't "/"

	return result;
}

std::string URLEncodePath(const std::string& strPath)
{
	using namespace std;
	vector<string> segments = StringUtils::Split(strPath, "/");
	for (vector<string>::iterator i = segments.begin(); i != segments.end(); ++i)
		*i = CUrl::Encode(*i);

	return StringUtils::Join(segments, "/");
}

std::string URLDecodePath(const std::string& strPath)
{
	using namespace std;
	vector<string> segments = StringUtils::Split(strPath, "/");
	for (vector<string>::iterator i = segments.begin(); i != segments.end(); ++i)
		*i = CUrl::Decode(*i);

	return StringUtils::Join(segments, "/");
}

std::string UrlUtils::ChangeBasePath(const std::string &fromPath, const std::string &fromFile, const std::string &toPath)
{
	std::string toFile = fromFile;

	// Convert back slashes to forward slashes, if required
	if (IsDOSPath(fromPath) && !IsDOSPath(toPath))
		StringUtils::Replace(toFile, "\\", "/");

	// Handle difference in URL encoded vs. not encoded
	if (HasEncodedFilename(CUrl(fromPath))
		&& !HasEncodedFilename(CUrl(toPath)))
	{
		toFile = URLDecodePath(toFile); // Decode path
	}
	else if (!HasEncodedFilename(CUrl(fromPath))
		&& HasEncodedFilename(CUrl(toPath)))
	{
		toFile = URLEncodePath(toFile); // Encode path
	}

	// Convert forward slashes to back slashes, if required
	if (!IsDOSPath(fromPath) && IsDOSPath(toPath))
		StringUtils::Replace(toFile, "/", "\\");

	return AddFileToFolder(toPath, toFile);
}


std::string UrlUtils::GetDirectory(const std::string &strFilePath)
{
	// Will from a full filename return the directory the file resides in.
	// Keeps the final slash at end and possible |option=foo options.

	size_t iPosSlash = strFilePath.find_last_of("/\\");
	if (iPosSlash == std::string::npos)
		return ""; // No slash, so no path (ignore any options)

	size_t iPosBar = strFilePath.rfind('|');
	if (iPosBar == std::string::npos)
		return strFilePath.substr(0, iPosSlash + 1); // Only path

	return strFilePath.substr(0, iPosSlash + 1) + strFilePath.substr(iPosBar); // Path + options
}