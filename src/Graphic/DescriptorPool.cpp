#include <Aka/Graphic/DescriptorPool.h>
#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {


DescriptorPool::DescriptorPool(const char* name, const ShaderBindingState& bindings, uint32_t size) :
	Resource(name, ResourceType::DescriptorPool),
	size(size),
	bindings(bindings)
{
}

};
};