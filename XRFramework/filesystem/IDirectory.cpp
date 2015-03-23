#include "stdafxf.h"
#include "IDirectory.h"
#include "utils/StringUtils.h"
#include "utils/UrlUtils.h"


IDirectory::IDirectory()
{
	m_flags = DIR_FLAG_DEFAULTS;
}


IDirectory::~IDirectory()
{
}

/*!
\brief Test if file have an allowed extension, as specified with SetMask()
\param strFile File to test
\return \e true if file is allowed
\note If extension is ".ifo", filename format must be "vide_ts.ifo" or
"vts_##_0.ifo". If extension is ".dat", filename format must be
"AVSEQ##(#).DAT", "ITEM###(#).DAT" or "MUSIC##(#).DAT".
*/
bool IDirectory::IsAllowed(const CURL& url) const
{
	if (m_strFileMask.empty())
		return true;

	// Check if strFile have an allowed extension
	if (!UrlUtils::HasExtension(url, m_strFileMask))
		return false;

	return true;
}

/*!
\brief Set a mask of extensions for the files in the directory.
\param strMask Mask of file extensions that are allowed.

The mask has to look like the following: \n
\verbatim
.m4a|.flac|.aac|
\endverbatim
So only *.m4a, *.flac, *.aac files will be retrieved with GetDirectory().
*/
void IDirectory::SetMask(const std::string& strMask)
{
	m_strFileMask = strMask;
	// ensure it's completed with a | so that filtering is easy.
	StringUtils::ToLower(m_strFileMask);
	if (m_strFileMask.size() && m_strFileMask[m_strFileMask.size() - 1] != '|')
		m_strFileMask += '|';
}

/*!
\brief Set the flags for this directory handler.
\param flags - \sa XFILE::DIR_FLAG for a description.
*/
void IDirectory::SetFlags(int flags)
{
	m_flags = flags;
}