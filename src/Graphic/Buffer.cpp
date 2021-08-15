#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/GraphicBackend.h>

namespace aka {

Buffer::Buffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access) :
	m_size(size),
	m_type(type),
	m_usage(usage),
	m_access(access)
{
}

Buffer::~Buffer()
{
}

Buffer::Ptr Buffer::create(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return GraphicBackend::createBuffer(type, size, usage, access, data);
}

size_t Buffer::size() const
{
	return m_size;
}

BufferType Buffer::type() const
{
	return m_type;
}

BufferUsage Buffer::usage() const
{
	return m_usage;
}

BufferCPUAccess Buffer::access() const
{
	return m_access;
}

}