#include <Aka/Graphic/Buffer.h>

#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

BufferHandle Buffer::createIndexBuffer(uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return Application::app()->graphic()->createBuffer(BufferType::Index, size, usage, access, data);
}

BufferHandle Buffer::createVertexBuffer(uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return Application::app()->graphic()->createBuffer(BufferType::Vertex, size, usage, access, data);
}

BufferHandle Buffer::createUniformBuffer(uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return Application::app()->graphic()->createBuffer(BufferType::Uniform, size, usage, access, data);
}

BufferHandle Buffer::createStagingBuffer(uint32_t size, const void* data)
{
	return BufferHandle::null;
}

void Buffer::destroy(BufferHandle buffer)
{
	return Application::app()->graphic()->destroy(buffer);
}

};
};