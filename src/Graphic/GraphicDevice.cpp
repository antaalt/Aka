#include <Aka/Graphic/GraphicDevice.h>

#include "Vulkan/VulkanGraphicDevice.h"

namespace aka {
namespace gfx {


GraphicDevice* GraphicDevice::create(GraphicAPI api)
{
	switch (api)
	{
#if defined(AKA_USE_VULKAN)
	case GraphicAPI::Vulkan:
		return mem::akaNew<VulkanGraphicDevice>(AllocatorMemoryType::Persistent, AllocatorCategory::Graphic);
#endif
	default:
		return nullptr;
	}
}

void GraphicDevice::destroy(GraphicDevice* device)
{
	mem::akaDelete(device);
}

};
};