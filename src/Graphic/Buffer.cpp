#include <Aka/Graphic/Buffer.h>

#include <Aka/Core/Application.h>

namespace aka {

Buffer* Buffer::createIndexBuffer(uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return Application::app()->graphic()->createBuffer(BufferType::Index, size, usage, access, data);
}

Buffer* Buffer::createVertexBuffer(uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return Application::app()->graphic()->createBuffer(BufferType::Vertex, size, usage, access, data);
}

Buffer* Buffer::createUniformBuffer(uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return Application::app()->graphic()->createBuffer(BufferType::Uniform, size, usage, access, data);
}

Buffer* Buffer::createStagingBuffer(uint32_t size, const void* data)
{
	return nullptr;
}

void Buffer::destroy(Buffer* buffer)
{
	return Application::app()->graphic()->destroy(buffer);
}

};