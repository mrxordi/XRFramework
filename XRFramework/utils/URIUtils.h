/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include "StdString.h"

class CURL;

class URIUtils
{
public:
	URIUtils(void);
	virtual ~URIUtils(void);

	static bool IsURL(const std::string& strFile);
	static bool IsURL(const CStdString& strFile);

	static bool IsInternetStream(const std::string& path, bool bStrictCheck = false);
	static bool IsInternetStream(const CURL& url, bool bStrictCheck = false);

	static bool IsDOSPath(const std::string& path);
	static bool IsDOSPath(const CStdString& path);

	static bool IsSpecial(const std::string& strFile);

	static bool IsProtocol(const std::string& url, const std::string &type);
	static bool IsHD(const CStdString& strFileName);

	static const std::string GetFileName(const std::string& strFileNameAndPath);
	static const std::string GetFileName(const CURL& url);


	static void AddSlashAtEnd(std::string& strFolder);
	static bool HasSlashAtEnd(const std::string& strFile, bool checkURL = false);
	static void RemoveSlashAtEnd(std::string& strFolder);
	static CStdString AddFileToFolder(const CStdString& strFolder, const CStdString& strFile);

	static bool HasParentInHostname(const CURL& url);
	static bool HasEncodedHostname(const CURL& url);
	static bool HasEncodedFilename(const CURL& url);

	static std::string GetExtension(const CURL& url);
	static std::string GetExtension(const std::string& strFileName);

	static std::string GetDirectory(const std::string &strFilePath);

	static std::string FixSlashesAndDups(const std::string& path, const char slashCharacter = '/', const size_t startFrom = 0);

	static std::string CanonicalizePath(const std::string& path, const char slashCharacter = '\\');

	static bool URIUtils::PathEquals(const std::string& url, const std::string &start) { return url == start;}

	static std::string ChangeBasePath(const std::string &fromPath, const std::string &fromFile, const std::string &toPath);

	/*!
	\brief Check if there is a file extension
	\param strFileName Path or URL to check
	\return \e true if strFileName have an extension.
	\note Returns false when strFileName is empty.
	\sa GetExtension
	*/
	static bool HasExtension(const std::string& strFileName);

	/*!
	\brief Check if filename have any of the listed extensions
	\param strFileName Path or URL to check
	\param strExtensions List of '.' prefixed lowercase extensions seperated with '|'
	\return \e true if strFileName have any one of the extensions.
	\note The check is case insensitive for strFileName, but requires
	strExtensions to be lowercase. Returns false when strFileName or
	strExtensions is empty.
	\sa GetExtension
	*/
	static bool HasExtension(const std::string& strFileName, const std::string& strExtensions);
	static bool HasExtension(const CURL& url, const std::string& strExtensions);

	static void RemoveExtension(std::string& strFileName);
	static std::string ReplaceExtension(const std::string& strFile,
		const std::string& strNewExtension);


	static const char* pictureExtensions;
	static const char* musicExtensions;
	static const char* videoExtensions;
	static const char* subtitlesExtensions;
};

