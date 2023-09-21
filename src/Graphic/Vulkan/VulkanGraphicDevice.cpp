#include "VulkanGraphicDevice.h"

#include <Aka/Memory/Allocator.h>
#include <Aka/OS/OS.h>

#if defined(AKA_USE_VULKAN)

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <renderdoc_app.h>

namespace aka {
namespace gfx {

VulkanGraphicDevice::VulkanGraphicDevice(PlatformDevice* platform, const GraphicConfig& cfg) :
	m_context(),
	m_swapchain()
{
#ifdef ENABLE_RENDERDOC_CAPTURE
	// Load renderdoc before any context creation.
	m_renderDocDll = OS::Link::load("C:/Program Files/RenderDoc/renderdoc.dll");
	// Try out some other basic path for renderdoc
	if (m_renderDocDll)
	{
		if (void* mod = OS::Link::open("renderdoc.dll"))
		{
			// https://renderdoc.org/docs/in_application_api.html
			pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)OS::Link::getProc(mod, "RENDERDOC_GetAPI");
			int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, (void**)&m_renderDocContext);
			AKA_ASSERT(ret == 1, "Failed to retrieve renderdoc dll");
			m_renderDocContext->SetCaptureFilePathTemplate("captures/aka");
			m_renderDocContext->SetCaptureOptionU32(eRENDERDOC_Option_CaptureCallstacks, true);
			m_renderDocContext->SetCaptureOptionU32(eRENDERDOC_Option_CaptureAllCmdLists, true);
			m_renderDocContext->SetCaptureOptionU32(eRENDERDOC_Option_SaveAllInitials, true);
			m_renderDocContext->MaskOverlayBits(eRENDERDOC_Overlay_None, eRENDERDOC_Overlay_None);
		}
	}
#endif
	m_context.initialize(platform, cfg);
	m_swapchain.initialize(this, platform);
}

VulkanGraphicDevice::~VulkanGraphicDevice()
{
	wait();
#ifdef ENABLE_RENDERDOC_CAPTURE
	OS::Link::free(m_renderDocDll);
#endif
	m_swapchain.shutdown(this);
	// Release all resources before destroying context.
	// We still check if resources where cleanly destroyed before releasing the remains.
	AKA_ASSERT(m_commandListPool.count() == 0, "Resource destroy missing");
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
	// TODO check recursive release issue
	m_commandListPool.release([this](VulkanCommandList& res) { this->release(&res); });
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
	//AKA_ASSERT(resource->native != ResourceNativeHandleInvalid, "Invalid native handle");
	//resource->type; // Get type & induce vktype with it
	//setDebugName<>(getVkDevice(), resource->native, name);
	AKA_NOT_IMPLEMENTED;
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
#ifdef ENABLE_RENDERDOC_CAPTURE
	if (m_renderDocContext && m_captureState == RenderDocCaptureState::PendingCapture)
	{
		m_captureState = RenderDocCaptureState::Capturing;
		m_renderDocContext->StartFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(getVkInstance()), m_context.getPlatform()->getNativeHandle());
		AKA_ASSERT(m_renderDocContext->IsFrameCapturing() == 1, "Frame not capturing...");
	}
#endif
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
		VkSemaphore signalSemaphore = vk_frame->semaphore[i + 1];
		VkSemaphore waitSemaphore = vk_frame->semaphore[i];
		VkFence fence = vk_frame->presentFence[i];
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmds[i];
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &waitSemaphore;

		VK_CHECK_RESULT(vkQueueSubmit(vk_queues[i], 1, &submitInfo, fence));
	}

	// Present
	SwapchainStatus status = m_swapchain.present(this, vk_frame);

#ifdef ENABLE_RENDERDOC_CAPTURE
	// Capture frame
	// https://renderdoc.org/docs/in_application_api.html
	if (m_renderDocContext && m_captureState == RenderDocCaptureState::Capturing)
	{
		AKA_ASSERT(m_renderDocContext->IsFrameCapturing() == 1, "Frame not capturing...");
		uint32_t ret = m_renderDocContext->EndFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(getVkInstance()), m_context.getPlatform()->getNativeHandle());
		if (ret == 1)
		{
			m_captureState = RenderDocCaptureState::Idle;
			ret = m_renderDocContext->GetCapture(m_renderDocContext->GetNumCaptures() - 1, NULL, NULL, NULL);
			if (ret == 1) // if capture is valid
			{
				// With 1 as first arg, renderdoc will connect to process immediately, 
				// so call ShowReplayUI after that.
				if (m_renderDocContext->IsTargetControlConnected())
				{
					m_renderDocContext->ShowReplayUI();
				}
				else
				{
					m_renderDocContext->LaunchReplayUI(1, "");
				}
			}
			else
			{
				Logger::error("Failed to retrieve renderdoc capture.");
			}
		}
		else
		{
			Logger::error("Renderdoc capture failed.");
		}
	}
#endif
	return status;
}

void VulkanGraphicDevice::wait()
{
	VK_CHECK_RESULT(vkDeviceWaitIdle(m_context.device));
}

void VulkanGraphicDevice::screenshot(void* data)
{
	AKA_NOT_IMPLEMENTED;
}

void VulkanGraphicDevice::capture()
{
#ifdef ENABLE_RENDERDOC_CAPTURE
	if (m_renderDocContext)
	{
		m_captureState = RenderDocCaptureState::PendingCapture;
	}
#endif
}

};
};

#endif
