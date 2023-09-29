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

DescriptorPoolHandle DescriptorPool::create(const char* name, const ShaderBindingState& bindings, uint32_t size)
{
	return Application::app()->graphic()->createDescriptorPool(name, bindings, size);
}

void DescriptorPool::destroy(DescriptorPoolHandle DescriptorPool)
{
	Application::app()->graphic()->destroy(DescriptorPool);
}

};
};