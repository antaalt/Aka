#include <Aka/OS/Stream/FileStream.h>

namespace aka {

FileMode operator&(FileMode lhs, FileMode rhs)
{
	return (FileMode)((int)lhs & (int)rhs);
}

FileMode operator|(FileMode lhs, FileMode rhs)
{
	return (FileMode)((int)lhs | (int)rhs);
}

FileStream::FileStream(const Path& path, FileMode mode, FileType type) :
	m_file(nullptr),
	m_mode(mode),
	m_length(0)
{
	m_file = fopen(path, mode, type);
	if (m_file == nullptr)
		return;
	m_mode = mode;
	fseek(m_file, 0L, SEEK_END);
	m_length = ftell(m_file);
 	::rewind(m_file);
}

FileStream::~FileStream()
{
	if (m_file)
		fclose(m_file);
}

void FileStream::skim(size_t size) 
{
	AKA_ASSERT(m_file != nullptr, "File not opened");
	int error = fseek(m_file, (long)size, SEEK_CUR);
	AKA_ASSERT(error == 0, "Error while seeking");
}

void FileStream::seek(size_t position)
{ 
	AKA_ASSERT(m_file != nullptr, "File not opened");
	int error = fseek(m_file, (long)position, SEEK_SET);
	AKA_ASSERT(error == 0, "Error while seeking");
}

size_t FileStream::size() const
{ 
	AKA_ASSERT(m_file != nullptr, "File not opened");
	return m_length;
}

size_t FileStream::offset()
{
	AKA_ASSERT(m_file != nullptr, "File not opened");
	return ftell(m_file);
}

void FileStream::rewind()
{ 
	AKA_ASSERT(m_file != nullptr, "File not opened");
	::rewind(m_file);
}

void FileStream::readData(void* data, size_t size)
{
	AKA_ASSERT((m_mode & FileMode::Read) == FileMode::Read, "File mode invalid");
	AKA_ASSERT(m_file != nullptr, "File not opened");
	size_t length = fread(data, 1, size, m_file);
	AKA_ASSERT(length == size, "File not read");
}

void FileStream::writeData(const void* data, size_t size)
{
	AKA_ASSERT((m_mode & FileMode::Write) == FileMode::Write, "File mode invalid");
	AKA_ASSERT(m_file != nullptr, "File not opened");
	size_t length = fwrite(data, 1, size, m_file);
	AKA_ASSERT(length == size, "File not written");
}

};