#include <Aka/Core/Container/Blob.h>

#include <cstring> // memcpy
#include <utility> // swap

namespace aka {

Blob::Blob() :
	m_bytes(nullptr),
	m_size(0)
{
}

Blob::Blob(size_t size) :
	m_bytes(mem::akaNewArray<byte_t>(size, AllocatorMemoryType::Blob, AllocatorCategory::Global)),
	m_size(size)
{
}

Blob::Blob(const void* bytes, size_t size) :
	m_bytes(mem::akaNewArray<byte_t>(size, AllocatorMemoryType::Blob, AllocatorCategory::Global)),
	m_size(size)
{
	memcpy(m_bytes, bytes, size);
}

Blob::Blob(const Blob& blob) :
	Blob(blob.data(), blob.size())
{
}

Blob& Blob::operator=(const Blob& blob)
{
	if (m_size != blob.m_size)
	{
		mem::akaDeleteArray(m_bytes);
		m_bytes = mem::akaNewArray<byte_t>(blob.m_size, AllocatorMemoryType::Blob, AllocatorCategory::Global);
		m_size = blob.m_size;
	}
	memcpy(m_bytes, blob.m_bytes, blob.m_size);
	return *this;
}

Blob::Blob(Blob&& blob) noexcept :
	m_bytes(nullptr),
	m_size(0)
{
	std::swap(m_bytes, blob.m_bytes);
	std::swap(m_size, blob.m_size);
}

Blob& Blob::operator=(Blob&& blob) noexcept
{
	std::swap(m_bytes, blob.m_bytes);
	std::swap(m_size, blob.m_size);
	return *this;
}

Blob::~Blob()
{
	mem::akaDeleteArray(m_bytes);
}

void* Blob::data()
{
	return m_bytes;
}

const void* Blob::data() const
{
	return m_bytes;
}

size_t Blob::size() const
{
	return m_size;
}

}; // namespace aka
