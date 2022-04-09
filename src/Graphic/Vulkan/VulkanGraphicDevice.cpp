#include "VulkanGraphicDevice.h"

#if defined(AKA_USE_VULKAN)

namespace aka {

VulkanGraphicDevice::VulkanGraphicDevice(PlatformDevice* platform, const GraphicConfig& cfg) :
	m_context(),
	m_swapchain()
{
	m_context.initialize(platform, cfg);
	m_swapchain.initialize(this, platform);
}

VulkanGraphicDevice::~VulkanGraphicDevice()
{
	m_swapchain.shutdown(this);
	m_context.shutdown();
}

GraphicAPI VulkanGraphicDevice::api() const
{
	return GraphicAPI::Vulkan;
}

uint32_t VulkanGraphicDevice::getPhysicalDeviceCount()
{
	return m_context.getPhysicalDeviceCount();
}

PhysicalDevice* VulkanGraphicDevice::getPhysicalDevice(uint32_t index)
{
	return m_context.getPhysicalDevice(index);
}

Frame* VulkanGraphicDevice::frame()
{
	// TODO wait then resize if require resize (shutdown and recreate)
	Frame* frame = m_swapchain.acquireNextImage(this);
	if (frame == nullptr)
	{
		Logger::error("Failed to acquire next swapchain image.");
		return nullptr;
	}
	frame->commandList = acquireCommandList();
	frame->commandList->begin();
	return frame;
}

void VulkanGraphicDevice::present(Frame* frame)
{
	// TODO wait then resize if require resize
	VulkanFrame* vk_frame = reinterpret_cast<VulkanFrame*>(frame);
	frame->commandList->end();
	submit(&vk_frame->commandList, 1);
	m_swapchain.present(this, vk_frame);
	release(vk_frame->commandList);
}

void VulkanGraphicDevice::screenshot(void* data)
{
	// TODO
}

};

#endif