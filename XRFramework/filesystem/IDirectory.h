#pragma once
#include <string>

class FileItemList;
class CUrl;

/*! \brief Available directory flags
The defaults are to allow file directories, no prompting, retrieve file information, hide hidden files, and utilise the directory cache
based on the implementation's wishes.
*/
enum DIR_FLAG
{
	DIR_FLAG_DEFAULTS = 0,
	DIR_FLAG_NO_FILE_DIRS = (2 << 0), ///< Don't convert files (zip, rar etc.) to directories
	DIR_FLAG_ALLOW_PROMPT = (2 << 1), ///< Allow prompting for further info (passwords etc.)
	DIR_FLAG_NO_FILE_INFO = (2 << 2), ///< Don't read additional file info (stat for example)
	DIR_FLAG_GET_HIDDEN = (2 << 3), ///< Get hidden files
	DIR_FLAG_READ_CACHE = (2 << 4), ///< Force reading from the directory cache (if available)
	DIR_FLAG_BYPASS_CACHE = (2 << 5)  ///< Completely bypass the directory cache (no reading, no writing)
};

class IDirectory
{
public:
	IDirectory(void);
	virtual ~IDirectory(void);

	/*!
	\brief Get the \e items of the directory \e strPath.
	\param url Directory to read.
	\param items Retrieves the directory entries.
	\return Returns \e true, if successfull.
	\sa CDirectoryFactory
	*/
	virtual bool GetDirectory(const CUrl& url, FileItemList &items) = 0;
	/*!
	\brief Retrieve the progress of the current directory fetch (if possible).
	\return the progress as a float in the range 0..100
	\sa GetDirectory, CancelDirectory
	*/
	virtual float GetProgress() const { return 0.0f; };
	/*!
	\brief Cancel the current directory fetch (if possible).
	\sa GetDirectory
	*/
	virtual void CancelDirectory() { };
	/*!
	\brief Create the directory
	\param url Directory to create.
	\return Returns \e true, if directory is created or if it already exists
	\sa CDirectoryFactory
	*/
	virtual bool Create(const CUrl& url) { return false; }
	/*!
	\brief Check for directory existence
	\param url Directory to check.
	\return Returns \e true, if directory exists
	\sa CDirectoryFactory
	*/
	virtual bool Exists(const CUrl& url) { return false; }
	/*!
	\brief Removes the directory
	\param url Directory to remove.
	\return Returns \e false if not succesfull
	*/
	virtual bool Remove(const CUrl& url) { return false; }

	/*!
	\brief Whether this file should be listed
	\param url File to test.
	\return Returns \e true if the file should be listed
	*/
	virtual bool IsAllowed(const CUrl& url) const;

	/*! \brief Whether to allow all files/folders to be listed.
	\return Returns \e true if all files/folder should be listed.
	*/
	virtual bool AllowAll() const { return false; }

	void SetMask(const std::string& strMask);
	void SetFlags(int flags);

protected:
	std::string m_strFileMask;  ///< Holds the file mask specified by SetMask()

	int m_flags; ///< Directory flags - see DIR_FLAG

};

