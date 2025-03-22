#include <Aka/Graphic/Instance.h>

#include "Vulkan/VulkanInstance.h"

namespace aka {
namespace gfx {
Instance* Instance::create(GraphicAPI api)
{
	switch (api)
	{
#if defined(AKA_USE_VULKAN)
	case GraphicAPI::Vulkan:
		return mem::akaNew<VulkanInstance>(AllocatorMemoryType::Object, AllocatorCategory::Graphic);
#endif
	default:
		return nullptr;
	}
}
void Instance::destroy(Instance* instance) 
{
	mem::akaDelete(instance);
}
}
}