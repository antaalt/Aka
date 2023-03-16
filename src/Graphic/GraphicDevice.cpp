#include <Aka/Graphic/GraphicDevice.h>

#include "Vulkan/VulkanGraphicDevice.h"

namespace aka {
namespace gfx {


GraphicDevice* GraphicDevice::create(PlatformDevice* device, const GraphicConfig& cfg)
{
	switch (cfg.api)
	{
	case GraphicAPI::Vulkan:
		return new VulkanGraphicDevice(device, cfg);
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