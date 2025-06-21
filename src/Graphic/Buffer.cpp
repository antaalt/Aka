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

};
};