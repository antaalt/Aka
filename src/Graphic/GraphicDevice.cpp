#include <Aka/Graphic/GraphicDevice.h>

#include "Vulkan/VulkanGraphicDevice.h"

namespace aka {
namespace gfx {

void Frame::setImageIndex(ImageIndex index)
{
	m_image = index;
}
void Frame::begin(GraphicDevice* device)
{
	m_commandList = device->acquireCommandList(); // TODO: Should reuse same command buffer.
	m_commandList->begin();
}
void Frame::end(GraphicDevice* device)
{
	m_commandList->end();
	device->submit(m_commandList, QueueType::Default);
}

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