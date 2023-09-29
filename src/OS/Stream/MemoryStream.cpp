#include <Aka/OS/Stream/MemoryStream.h>

#include <cstring>

namespace aka {

MemoryStream::MemoryStream(size_t size) :
	m_offset(0),
	m_size(size)
{
}

MemoryReaderStream::MemoryReaderStream(const uint8_t* bytes, size_t size) :
	MemoryStream(size),
	m_bytes(bytes)
{
}

MemoryReaderStream::MemoryReaderStream(const Vector<uint8_t>& bytes) :
	MemoryReaderStream(bytes.data(), bytes.size())
{
}

MemoryReaderStream::MemoryReaderStream(const Blob& blob) :
	MemoryReaderStream(reinterpret_cast<const uint8_t*>(blob.data()), blob.size())
{
}
MemoryReaderStream::~MemoryReaderStream()
{
}

MemoryWriterStream::MemoryWriterStream(Vector<uint8_t>& bytes) :
	MemoryStream(bytes.size()),
	m_bytes(bytes)
{
}
MemoryWriterStream::~MemoryWriterStream()
{
}

void MemoryStream::skim(size_t count) 
{ 
	m_offset += count; 
}

void MemoryStream::seek(size_t position)
{ 
	m_offset = position; 
}

size_t MemoryStream::size() const
{
	return m_size; 
}

size_t MemoryStream::offset()
{ 
	return m_offset; 
}

void MemoryStream::rewind()
{
	m_offset = 0;
}


void MemoryReaderStream::read(void* data, size_t size)
{
	AKA_ASSERT(m_offset + size >= m_size, "Reading out of bounds");
	Memory::copy(data, m_bytes + m_offset, size);
	m_offset += size;
}

void MemoryReaderStream::write(const void* data, size_t size)
{
	AKA_ASSERT(false, "Cannot write with a reader stream.");
}

void MemoryWriterStream::read(void* data, size_t size)
{
	AKA_ASSERT(false, "Cannot write with a reader stream.");
}

void MemoryWriterStream::write(const void* data, size_t size)
{
	const uint8_t* datau = static_cast<const uint8_t*>(data);
	m_bytes.append(datau, datau + size);
	m_size = m_bytes.size();
}


};