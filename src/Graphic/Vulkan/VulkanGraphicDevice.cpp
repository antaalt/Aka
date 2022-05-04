#include "VulkanGraphicDevice.h"

#if defined(AKA_USE_VULKAN)

namespace aka {
namespace gfx {

VulkanGraphicDevice::VulkanGraphicDevice(PlatformDevice* platform, const GraphicConfig& cfg) :
	m_context(),
	m_swapchain()
{
	m_context.initialize(platform, cfg);
	m_swapchain.initialize(this, platform);
}

VulkanGraphicDevice::~VulkanGraphicDevice()
{
	// Release all resources before destroying context.
	//AKA_ASSERT(m_texturePool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_samplerPool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_bufferPool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_shaderPool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_programPool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_framebufferPool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_pipelinePool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_descriptorPool.count() == 0, "Resource destroy missing");
	//m_texturePool.release([this](const VulkanTexture& res) { this->destroy(const_cast<VulkanTexture*>(&res)); });
	m_samplerPool.release([this](const VulkanSampler& res) { this->destroy(const_cast<VulkanSampler*>(&res)); });
	m_bufferPool.release([this](const VulkanBuffer& res) { this->destroy(const_cast<VulkanBuffer*>(&res)); });
	m_shaderPool.release([this](const VulkanShader& res) { this->destroy(const_cast<VulkanShader*>(&res)); });
	m_programPool.release([this](const VulkanProgram& res) { this->destroy(const_cast<VulkanProgram*>(&res)); });
	m_framebufferPool.release([this](const VulkanFramebuffer& res) { this->destroy(const_cast<VulkanFramebuffer*>(&res)); });
	m_pipelinePool.release([this](const VulkanPipeline& res) { this->destroy(const_cast<VulkanPipeline*>(&res)); });
	//m_descriptorPool.release([this](const VulkanDescriptorSet& res) { this->destroy(const_cast<VulkanDescriptorSet*>(&res)); });
	// Destroy context
	m_swapchain.shutdown(this);
	m_context.shutdown();
}

GraphicAPI VulkanGraphicDevice::api() const
{
	return GraphicAPI::Vulkan;
}

void VulkanGraphicDevice::name(const Resource* resource, const char* name)
{
	VkDebugUtilsObjectNameInfoEXT nameInfo{};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
	nameInfo.pNext = NULL;
	nameInfo.objectType = VK_OBJECT_TYPE_UNKNOWN; // TODO
	nameInfo.objectHandle = resource->native;
	nameInfo.pObjectName = name;
	vkSetDebugUtilsObjectNameEXT(m_context.device, &nameInfo);

	String::copy(const_cast<Resource*>(resource)->name, sizeof(resource->name), name);
}

uint32_t VulkanGraphicDevice::getPhysicalDeviceCount()
{
	return m_context.getPhysicalDeviceCount();
}

const PhysicalDevice* VulkanGraphicDevice::getPhysicalDevice(uint32_t index)
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
};

#endif
