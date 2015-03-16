#pragma once
#include "filesystem/IFile.h"
#include "Base.h"
#include <string>

typedef void* HANDLE; // forward declaration

class CWin32File : public IFile
{
public:
	CWin32File();
	virtual ~CWin32File();

	virtual bool Open(const CUrl& url);
	virtual bool OpenForWrite(const CUrl& url, bool bOverWrite = false);
	virtual void Close();

	virtual ssize_t Read(void* lpBuf, int64_t uiBufSize);
	virtual ssize_t Write(const void* lpBuf, int64_t uiBufSize);
	virtual int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET);
	virtual int Truncate(int64_t toSize);
	virtual int64_t GetPosition();
	virtual int64_t GetLength();
	virtual void Flush();

	virtual bool Delete(const CUrl& url);
	virtual bool Rename(const CUrl& urlCurrentName, const CUrl& urlNewName);
	virtual bool SetHidden(const CUrl& url, bool hidden);
	virtual bool Exists(const CUrl& url);
	virtual int Stat(const CUrl& url, struct __stat64* statData);
	virtual int Stat(struct __stat64* statData);

protected:
	CWin32File(bool asSmbFile);
	HANDLE  m_hFile;
	int64_t m_filePos;
	bool    m_allowWrite;
	// file path and name in win32 long form "\\?\D:\path\to\file.ext"
	std::wstring m_filepathnameW;
	const bool m_smbFile; // true for SMB file, false for local file
	unsigned long m_lastSMBFileErr; // used for SMB file operations
};