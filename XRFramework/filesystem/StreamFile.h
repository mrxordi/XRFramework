#pragma once
#include <iostream>
#include "Base.h"

class IFile;
class CURL;

using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
// CFileStreamBuffer // streambuf for file io, only supports buffered input currently
/////////////////////////////////////////////////////////////////////////////////////////
class CFileStreamBuffer : public std::streambuf
{
public:
	~CFileStreamBuffer();
	CFileStreamBuffer(int backsize = 0);

	void Attach(IFile *file);
	void Detach();

private:
	virtual int_type underflow();
	virtual std::streamsize showmanyc();
	virtual pos_type seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode = std::ios_base::in | std::ios_base::out);
	virtual pos_type seekpos(pos_type, std::ios_base::openmode = std::ios_base::in | std::ios_base::out);

	IFile* m_file;
	char*  m_buffer;
	int    m_backsize;
	int    m_frontsize;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CFileStreamBuffer // streambuf for file io, only supports buffered input currently
/////////////////////////////////////////////////////////////////////////////////////////
// very basic file input stream
class CFileStream : public std::istream
{
public:
	CFileStream(int backsize = 0);
	~CFileStream();

	bool Open(const std::string& filename);
	bool Open(const CURL& filename);
	void Close();

	int64_t GetLength();
private:
	CFileStreamBuffer m_buffer;
	IFile*            m_file;
};
