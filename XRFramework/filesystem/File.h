#pragma once
#include "IFile.h"
#include "StreamFile.h"
#include "XRCommon/utils/URL.h"
#include "utils/AutoBuffer.h"

class CFileStreamBuffer;

class CFile
{
public:
	CFile();
	CFile(const std::string& path, const unsigned int flags = 0);
	CFile(const CURL& curl, const unsigned int flags = 0);
	virtual ~CFile();

	bool Open();
	void Close();

	bool Open(const CURL& file, const unsigned int flags = 0);
	bool Open(const std::string& strFileName, const unsigned int flags = 0);

	bool OpenForWrite(const std::string& strFileName, bool bOverWrite = false);
	bool OpenForWrite(const CURL& file, bool bOverWrite = false);
	bool OpenForWrite(bool bOverWrite = false);

	ssize_t LoadFile(const std::string& file, auto_buffer& outputBuffer);
	ssize_t LoadFile(const CURL& file, auto_buffer& outputBuffer);
	ssize_t LoadFile(auto_buffer& outputBuffer);

	/*read bufSize bytes from currently opened file into buffer bufPtr.*/
	ssize_t Read(void* bufPtr, size_t bufSize);
	bool ReadString(char *szLine, int iLineLength);

	/*Write bufSize bytes from buffer bufPtr into currently opened file.*/
	ssize_t Write(const void* bufPtr, size_t bufSize);

	void Flush();
	int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET);
	int Truncate(int64_t iSize);
	int64_t GetPosition() const;
	int64_t GetLength();
	bool Exist();
	bool Delete();
	bool Rename(const std::string& newFileName);
	/**
	* Fills struct __stat64 with information about currently open file
	* For st_mode function will set correctly _S_IFDIR (directory) flag and may set
	* _S_IREAD (read permission), _S_IWRITE (write permission) flags if such
	* information is available. Function may set st_size (file size), st_atime,
	* st_mtime, st_ctime (access, modification, creation times).
	*/
	int Stat(struct __stat64 *buffer);
	int GetChunkSize();
	std::string GetContentMimeType(void);
	std::string GetContentCharset(void);
	IFile *GetImplemenation() { return m_pFile; }

	//BitstreamStats* GetBitstreamStats() { return m_bitStreamStats; }
	int IoControl(EIoControl request, void* param);
#pragma region CURL interface
public:
	static int GetChunkSize(int chunk, int minimum)
	{
		if (chunk)
			return chunk * ((minimum + chunk - 1) / chunk);
		else
			return minimum;
	}

	// CURL interface
	static bool Exists(const CURL& file, bool bUseCache = true);
	static bool Delete(const CURL& file);
	/**
	* Fills struct __stat64 with information about file specified by filename
	* For st_mode function will set correctly _S_IFDIR (directory) flag and may set
	* _S_IREAD (read permission), _S_IWRITE (write permission) flags if such
	* information is available. Function may set st_size (file size), st_atime,
	* st_mtime, st_ctime (access, modification, creation times).
	* Any other flags and members of __stat64 that didn't updated with actual file
	* information will be set to zero (st_nlink can be set ether to 1 or zero).
	*/
	static int  Stat(const CURL& file, struct __stat64* buffer);
	static bool Rename(const CURL& file, const CURL& urlNew);
	static bool Copy(const CURL& file, const CURL& dest, IFileCallback* pCallback = NULL, void* pContext = NULL);
	static bool SetHidden(const CURL& file, bool hidden);
	static bool Exists(const std::string& strFileName, bool bUseCache = true);
	static int  Stat(const std::string& strFileName, struct __stat64* buffer);
	static bool Delete(const std::string& strFileName);
	static bool Rename(const std::string& strFileName, const std::string& strNewFileName);
	static bool Copy(const std::string& strFileName, const std::string& strDest, IFileCallback* pCallback = NULL, void* pContext = NULL);
	static bool SetHidden(const std::string& fileName, bool hidden);
#pragma endregion

	static IFile* FileFactory(const std::string& strFileName);
	static IFile* FileFactory(CURL& url);
private:
	unsigned int m_flags;
	IFile* m_pFile;
	CURL m_curl;
	std::string m_mimetype;

	CFileStreamBuffer* m_pBuffer;
	//BitstreamStats* m_bitStreamStats;
};