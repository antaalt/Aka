#include <Aka/OS/Stream/MemoryStream.h>

#include <cstring>

namespace aka {

MemoryStream::MemoryStream(const uint8_t* bytes, size_t size) :
	Stream(),
	m_bytes(bytes), 
	m_bytesWrite(nullptr), 
	m_size(size), 
	m_offset(0)
{
}

MemoryStream::MemoryStream(const Vector<uint8_t>& bytes) : 
	MemoryStream(bytes.data(), bytes.size()) 
{
}

MemoryStream::MemoryStream(const Blob& blob) :
	MemoryStream(reinterpret_cast<const uint8_t*>(blob.data()), blob.size())
{
}

MemoryStream::MemoryStream(uint8_t* bytes, size_t size) :
	Stream(),
	m_bytes(bytes), 
	m_bytesWrite(bytes),
	m_size(size), 
	m_offset(0) 
{
}

MemoryStream::MemoryStream(Vector<uint8_t>& bytes) : 
	MemoryStream(bytes.data(), bytes.size()) 
{
}

MemoryStream::MemoryStream(Blob& blob) :
	MemoryStream(reinterpret_cast<uint8_t*>(blob.data()), blob.size())
{
}

MemoryStream::~MemoryStream() 
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

const uint8_t* MemoryStream::data(size_t len) const
{ 
	return m_bytes + m_offset; 
}

void MemoryStream::unserialize(void* data, size_t size)
{
	if (m_offset + size > m_size)
		throw std::runtime_error("Reading out of bounds");
	memcpy(data, m_bytes + m_offset, size);
	m_offset += size;
}

void MemoryStream::serialize(const void* data, size_t size)
{
	if (m_offset + size > m_size)
		throw std::runtime_error("Writing out of bounds");
	memcpy(m_bytesWrite + m_offset, data, size);
	m_offset += size;
}

};