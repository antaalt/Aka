#include <Aka/OS/Stream/FileStream.h>

namespace aka {

FileStream::FileStream(const Path& path, FileMode mode) : 
	m_file(path, mode) 
{
}

FileStream::~FileStream()
{
}

void FileStream::skim(size_t size) 
{
	AKA_ASSERT(m_file.opened(), "File not opened");
	m_file.seek(m_file.position() + size); 
}

void FileStream::seek(size_t position)
{ 
	AKA_ASSERT(m_file.opened(), "File not opened");
	m_file.seek(position);
}

size_t FileStream::size() const
{ 
	AKA_ASSERT(m_file.opened(), "File not opened");
	return m_file.length();
}

size_t FileStream::offset()
{
	AKA_ASSERT(m_file.opened(), "File not opened");
	return m_file.position();
}

void FileStream::rewind()
{ 
	AKA_ASSERT(m_file.opened(), "File not opened");
	m_file.seek(0);
}

void FileStream::readData(void* data, size_t size)
{
	AKA_ASSERT((m_file.mode() & FileMode::Read) == FileMode::Read, "File mode invalid");
	AKA_ASSERT(m_file.opened(), "File not opened");
	m_file.read(data, size);
}

void FileStream::writeData(const void* data, size_t size)
{
	AKA_ASSERT((m_file.mode() & FileMode::Write) == FileMode::Write, "File mode invalid");
	AKA_ASSERT(m_file.opened(), "File not opened");
	m_file.write(data, size);
}

};