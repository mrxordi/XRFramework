#include "stdafxf.h"
#include "StreamFile.h"
#include "IFile.h"
#include "File.h"
#include "XRCommon/utils/URL.h"


using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
// CFileStreamBuffer // streambuf for file io, only supports buffered input currently
/////////////////////////////////////////////////////////////////////////////////////////
CFileStreamBuffer::~CFileStreamBuffer()
{
	sync();
	Detach();
}

CFileStreamBuffer::CFileStreamBuffer(int backsize)
	: streambuf()
	, m_file(NULL)
	, m_buffer(NULL)
	, m_backsize(backsize)
	, m_frontsize(0)
{
}

void CFileStreamBuffer::Attach(IFile *file)
{
	m_file = file;
	m_frontsize = CFile::GetChunkSize(m_file->GetChunkSize(), 64 * 1024);

	m_buffer = new char[m_frontsize + m_backsize];
	setg(0, 0, 0);
	setp(0, 0);
}

void CFileStreamBuffer::Detach()
{
	setg(0, 0, 0);
	setp(0, 0);
	delete[] m_buffer;
	m_buffer = NULL;
}

CFileStreamBuffer::int_type CFileStreamBuffer::underflow()
{
	if (gptr() < egptr())
		return traits_type::to_int_type(*gptr());

	if (!m_file)
		return traits_type::eof();

	size_t backsize = 0;
	if (m_backsize)
	{
		backsize = (size_t)min<ptrdiff_t>((ptrdiff_t)m_backsize, egptr() - eback());
		memmove(m_buffer, egptr() - backsize, backsize);
	}

	unsigned int size = m_file->Read(m_buffer + backsize, m_frontsize);

	if (size == 0)
		return traits_type::eof();

	setg(m_buffer, m_buffer + backsize, m_buffer + backsize + size);
	return traits_type::to_int_type(*gptr());
}

CFileStreamBuffer::pos_type CFileStreamBuffer::seekoff(
	off_type offset,
	ios_base::seekdir way,
	ios_base::openmode mode)
{
	// calculate relative offset
	off_type pos = m_file->GetPosition() - (egptr() - gptr());
	off_type offset2;
	if (way == ios_base::cur)
		offset2 = offset;
	else if (way == ios_base::beg)
		offset2 = offset - pos;
	else if (way == ios_base::end)
		offset2 = offset + m_file->GetLength() - pos;
	else
		return streampos(-1);

	// a non seek shouldn't modify our buffer
	if (offset2 == 0)
		return pos;

	// try to seek within buffer
	if (gptr() + offset2 >= eback() && gptr() + offset2 < egptr())
	{
		gbump(offset2);
		return pos + offset2;
	}

	// reset our buffer pointer, will
	// start buffering on next read
	setg(0, 0, 0);
	setp(0, 0);

	int64_t position = -1;
	if (way == ios_base::cur)
		position = m_file->Seek(offset, SEEK_CUR);
	else if (way == ios_base::end)
		position = m_file->Seek(offset, SEEK_END);
	else
		position = m_file->Seek(offset, SEEK_SET);

	if (position < 0)
		return streampos(-1);

	return position;
}

CFileStreamBuffer::pos_type CFileStreamBuffer::seekpos(
	pos_type pos,
	ios_base::openmode mode)
{
	return seekoff(pos, ios_base::beg, mode);
}

streamsize CFileStreamBuffer::showmanyc()
{
	underflow();
	return egptr() - gptr();
}

/////////////////////////////////////////////////////////////////////////////////////////
// CFileStreamBuffer // streambuf for file io, only supports buffered input currently
/////////////////////////////////////////////////////////////////////////////////////////
// very basic file input stream
CFileStream::CFileStream(int backsize /*= 0*/) : istream(&m_buffer), m_buffer(backsize), m_file(NULL)
{
}

CFileStream::~CFileStream()
{
	Close();
}

bool CFileStream::Open(const CURL& filename)
{
	Close();

	CURL url(filename);
	m_file = CFile::FileFactory(url);
	if (m_file && m_file->Open(url))
	{
		m_buffer.Attach(m_file);
		return true;
	}

	setstate(failbit);
	return false;
}

int64_t CFileStream::GetLength()
{
	return m_file->GetLength();
}

void CFileStream::Close()
{
	if (!m_file)
		return;

	m_buffer.Detach();
	SAFE_DELETE(m_file);
}

bool CFileStream::Open(const std::string& filename)
{
	const CURL pathToUrl(filename);
	return Open(pathToUrl);
}
