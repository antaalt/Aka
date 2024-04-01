#include <Aka/Graphic/Buffer.h>

#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

Buffer::Buffer(const char* name, BufferType type, uint32_t size, BufferUsage usage, BufferCPUAccess access) :
	Resource(name, ResourceType::Buffer),
	type(type),
	size(size),
	usage(usage),
	access(access)
{
}

BufferHandle Buffer::createIndexBuffer(const char* name, uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return Application::app()->graphic()->createBuffer(name, BufferType::Index, size, usage, access, data);
}

BufferHandle Buffer::createVertexBuffer(const char* name, uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return Application::app()->graphic()->createBuffer(name, BufferType::Vertex, size, usage, access, data);
}

BufferHandle Buffer::createUniformBuffer(const char* name, uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return Application::app()->graphic()->createBuffer(name, BufferType::Uniform, size, usage, access, data);
}

BufferHandle Buffer::createStagingBuffer(const char* name, uint32_t size, const void* data)
{
	return BufferHandle::null;
}

void Buffer::destroy(BufferHandle buffer)
{
	return Application::app()->graphic()->destroy(buffer);
}

};
};