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
		return new VulkanGraphicDevice();
#endif
	default:
		return nullptr;
	}
}

void GraphicDevice::destroy(GraphicDevice* device)
{
	delete device;
}

};
};