#include "VulkanGraphicDevice.h"

#include <Aka/Memory/Allocator.h>

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
	wait();
	m_swapchain.shutdown(this);
	// Release all resources before destroying context.
	// We still check if resources where cleanly destroyed before releasing the remains.
	AKA_ASSERT(m_texturePool.count() == 0, "Resource destroy missing");
	AKA_ASSERT(m_samplerPool.count() == 0, "Resource destroy missing");
	AKA_ASSERT(m_bufferPool.count() == 0, "Resource destroy missing");
	AKA_ASSERT(m_shaderPool.count() == 0, "Resource destroy missing");
	AKA_ASSERT(m_programPool.count() == 0, "Resource destroy missing");
	AKA_ASSERT(m_framebufferPool.count() == 0, "Resource destroy missing");
	AKA_ASSERT(m_graphicPipelinePool.count() == 0, "Resource destroy missing");
	AKA_ASSERT(m_computePipelinePool.count() == 0, "Resource destroy missing");
	AKA_ASSERT(m_descriptorPool.count() == 0, "Resource destroy missing");
	AKA_ASSERT(m_renderPassPool.count() == 0, "Resource destroy missing");
	m_texturePool.release([this](VulkanTexture& res) { this->destroy(TextureHandle{ &res }); });
	m_samplerPool.release([this](VulkanSampler& res) { this->destroy(SamplerHandle{ &res }); });
	m_bufferPool.release([this](VulkanBuffer& res) { this->destroy(BufferHandle{ &res }); });
	m_shaderPool.release([this](VulkanShader& res) { this->destroy(ShaderHandle{ &res }); });
	m_programPool.release([this](VulkanProgram& res) { this->destroy(ProgramHandle{ &res }); });
	m_framebufferPool.release([this](VulkanFramebuffer& res) { this->destroy(FramebufferHandle{ &res }); });
	m_graphicPipelinePool.release([this](VulkanGraphicPipeline& res) { this->destroy(GraphicPipelineHandle{ &res }); });
	m_computePipelinePool.release([this](VulkanComputePipeline& res) { this->destroy(ComputePipelineHandle{ &res }); });
	m_descriptorPool.release([this](const VulkanDescriptorSet& res) { this->destroy(DescriptorSetHandle{ &res }); });
	m_renderPassPool.release([this](const VulkanRenderPass& res) { this->destroy(RenderPassHandle{ &res }); });
	// Destroy context
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
	VK_CHECK_RESULT(vkSetDebugUtilsObjectNameEXT(m_context.device, &nameInfo));

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
	VulkanFrame* vk_frame = m_swapchain.acquireNextImage(this);
	if (vk_frame == nullptr)
	{
		Logger::error("Failed to acquire next swapchain image.");
		return nullptr;
	}
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
		vk_frame->commandLists[i].begin();
	return vk_frame;
}

SwapchainStatus VulkanGraphicDevice::present(Frame* frame)
{
	VulkanFrame* vk_frame = reinterpret_cast<VulkanFrame*>(frame);

	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
		vk_frame->commandLists[i].end();

	// Submit
	VkQueue vk_queues[EnumCount<QueueType>()] = {
		getVkQueue(QueueType::Graphic),
		getVkQueue(QueueType::Compute),
		getVkQueue(QueueType::Copy)
	};

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

	VkCommandBuffer cmds[EnumCount<QueueType>()] = {
		vk_frame->commandLists[EnumToIndex(QueueType::Graphic)].vk_command,
		vk_frame->commandLists[EnumToIndex(QueueType::Compute)].vk_command,
		vk_frame->commandLists[EnumToIndex(QueueType::Copy)].vk_command,
	};
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		VkSemaphore signalSemaphore = vk_frame->presentSemaphore[i];
		VkSemaphore waitSemaphore = vk_frame->acquireSemaphore;
		VkFence fence = vk_frame->presentFence[i];
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmds[i];
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore; // Should have one per command
		if (i == 0) // HACK //
		{
			// For now, only graphic queue wait for acquire semaphore cause only one queue can wait for it.
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &waitSemaphore;
		}
		else
		{
			submitInfo.waitSemaphoreCount = 0;
			submitInfo.pWaitSemaphores = VK_NULL_HANDLE;
		}

		VK_CHECK_RESULT(vkQueueSubmit(vk_queues[i], 1, &submitInfo, fence));
	}
	// Present
	return m_swapchain.present(this, vk_frame);
}

void VulkanGraphicDevice::wait()
{
	VK_CHECK_RESULT(vkDeviceWaitIdle(m_context.device));
}

void VulkanGraphicDevice::screenshot(void* data)
{
	AKA_NOT_IMPLEMENTED;
}

};
};

#endif
