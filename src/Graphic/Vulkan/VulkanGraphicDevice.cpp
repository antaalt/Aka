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

VulkanGraphicDevice::VulkanGraphicDevice() :
	m_context(),
	m_swapchain()
{
}

VulkanGraphicDevice::~VulkanGraphicDevice()
{
}

bool VulkanGraphicDevice::initialize(PlatformDevice* platform, const GraphicConfig& cfg)
{
#ifdef ENABLE_RENDERDOC_CAPTURE
	if (asBool(cfg.features & PhysicalDeviceFeatures::RenderDocAttachment))
	{
		// Load renderdoc before any context creation.
		// TODO should use OS::Library::getLibraryPath();
		// TODO linux has different path.
		m_renderDocLibrary = OS::Library("C:/Program Files/RenderDoc/renderdoc.dll");
		if (m_renderDocLibrary.isLoaded())
		{
			// https://renderdoc.org/docs/in_application_api.html
			pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)m_renderDocLibrary.getProcess("RENDERDOC_GetAPI");
			int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, (void**)&m_renderDocContext);
			AKA_ASSERT(ret == 1, "Failed to retrieve renderdoc dll");
			// Generate unique path depending on date to avoid blocking apps.
			Date date = Date::localtime();
			const String capturePath = String::format("aka-captures/%4u-%2u-%2u/%2u-%2u-%2u/", date.year, date.month, date.day, date.hour, date.minute, date.second);
			m_renderDocContext->SetCaptureFilePathTemplate(capturePath.cstr());
			RENDERDOC_InputButton button = eRENDERDOC_Key_F11;
			m_renderDocContext->SetCaptureKeys(&button, 1);
			m_renderDocContext->SetCaptureOptionU32(eRENDERDOC_Option_CaptureCallstacks, true);
			m_renderDocContext->SetCaptureOptionU32(eRENDERDOC_Option_CaptureAllCmdLists, true);
			m_renderDocContext->SetCaptureOptionU32(eRENDERDOC_Option_SaveAllInitials, true);
			m_renderDocContext->SetCaptureOptionU32(eRENDERDOC_Option_RefAllResources, true);
			m_renderDocContext->SetCaptureOptionU32(eRENDERDOC_Option_APIValidation, true);
			m_renderDocContext->SetCaptureOptionU32(eRENDERDOC_Option_DebugOutputMute, false);
			m_renderDocContext->MaskOverlayBits(eRENDERDOC_Overlay_None, eRENDERDOC_Overlay_None);
		}
		else
		{
			Logger::error("Failed to load renderdoc library.");
		}
	}
#endif
	if (!m_context.initialize(platform, cfg))
		return false;
	return m_swapchain.initialize(this, platform);
}

void VulkanGraphicDevice::shutdown()
{
	wait();
	m_swapchain.shutdown(this);
	// Release all resources before destroying context.
	// We still check if resources where cleanly destroyed before releasing the remains.
	//AKA_ASSERT(m_commandListPool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_texturePool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_samplerPool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_bufferPool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_shaderPool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_programPool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_framebufferPool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_graphicPipelinePool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_computePipelinePool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_descriptorSetPool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_descriptorPoolPool.count() == 0, "Resource destroy missing");
	//AKA_ASSERT(m_renderPassPool.count() == 0, "Resource destroy missing");
	// TODO check recursive release issue
	m_commandListPool.release([this](VulkanCommandList& res) { Logger::warn("Leaking command list."); this->release(&res); });
	m_texturePool.release([this](VulkanTexture& res) { Logger::warn("Leaking texture ", res.name); this->destroy(TextureHandle{ &res }); });
	m_samplerPool.release([this](VulkanSampler& res) { Logger::warn("Leaking sampler ", res.name); this->destroy(SamplerHandle{ &res }); });
	m_bufferPool.release([this](VulkanBuffer& res) { Logger::warn("Leaking buffer ", res.name); this->destroy(BufferHandle{ &res }); });
	m_shaderPool.release([this](VulkanShader& res) { Logger::warn("Leaking shader ", res.name); this->destroy(ShaderHandle{ &res }); });
	m_programPool.release([this](VulkanProgram& res) { Logger::warn("Leaking program ", res.name); this->destroy(ProgramHandle{ &res }); });
	m_framebufferPool.release([this](VulkanFramebuffer& res) { Logger::warn("Leaking framebuffer", res.name); this->destroy(FramebufferHandle{ &res }); });
	m_graphicPipelinePool.release([this](VulkanGraphicPipeline& res) { Logger::warn("Leaking graphic pipeline", res.name); this->destroy(GraphicPipelineHandle{ &res }); });
	m_computePipelinePool.release([this](VulkanComputePipeline& res) { Logger::warn("Leaking compute pipeline", res.name); this->destroy(ComputePipelineHandle{&res}); });
	//m_descriptorSetPool.release([this](const VulkanDescriptorSet& res) { Logger::warn("Leaking descriptor set", res.name); this->free(DescriptorSetHandle{ &res }); });
	m_descriptorPoolPool.release([this](const VulkanDescriptorPool& res) { Logger::warn("Leaking desciptor pool", res.name); this->destroy(DescriptorPoolHandle{ &res }); });
	m_renderPassPool.release([this](const VulkanRenderPass& res) { Logger::warn("Leaking render pass ", res.name); this->destroy(RenderPassHandle{ &res }); });
	// Destroy context
	m_context.shutdown();
}

GraphicAPI VulkanGraphicDevice::getApi() const
{
	return GraphicAPI::Vulkan;
}
PhysicalDeviceFeatures VulkanGraphicDevice::getFeatures() const
{
	return m_context.physicalDeviceFeatures;
}
bool VulkanGraphicDevice::hasFeatures(PhysicalDeviceFeatures _features) const
{
	return asBool(m_context.physicalDeviceFeatures & _features);
}
PhysicalDeviceLimits VulkanGraphicDevice::getLimits() const
{
	return m_context.physicalDeviceLimits;
}

CommandList* VulkanGraphicDevice::acquireCommandList(FrameHandle frame, QueueType queue)
{
	VulkanFrame& vk_frame = m_swapchain.getVkFrame(frame);
	return vk_frame.allocateCommand(this, queue);
}

void VulkanGraphicDevice::release(FrameHandle frame, CommandList* cmd)
{
	VulkanFrame& vk_frame = m_swapchain.getVkFrame(frame);
	return vk_frame.releaseCommand(reinterpret_cast<VulkanCommandList*>(cmd));
}

FrameHandle VulkanGraphicDevice::frame()
{
#ifdef ENABLE_RENDERDOC_CAPTURE
	if (m_renderDocContext && m_captureState == RenderDocCaptureState::PendingCapture)
	{
		m_captureState = RenderDocCaptureState::Capturing;
		m_renderDocContext->StartFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(getVkInstance()), m_context.getPlatform()->getNativeHandle());
		AKA_ASSERT(m_renderDocContext->IsFrameCapturing() == 1, "Frame not capturing...");
	}
#endif
	VulkanFrame* vk_frame = m_swapchain.acquireNextImage(this);
	if (vk_frame == nullptr)
	{
		Logger::error("Failed to acquire next swapchain image.");
		return FrameHandle::null;
	}
	static const char* s_commandName[EnumCount<QueueType>()] = {
		"Graphic main command list",
		"Compute main command list",
		"Copy main command list",
	};
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		color4f markerColor(0.6f, 0.6f, 0.6f, 1.f);
		vk_frame->mainCommandLists[i].begin();
		vk_frame->mainCommandLists[i].beginMarker(s_commandName[i], markerColor.data);
	}
	return FrameHandle{ vk_frame };
}

SwapchainStatus VulkanGraphicDevice::present(FrameHandle frame)
{
	VulkanFrame& vk_frame = m_swapchain.getVkFrame(frame);

	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		vk_frame.mainCommandLists[i].endMarker();
		vk_frame.mainCommandLists[i].end();
	}

	// Submit
	VkQueue vk_queues[EnumCount<QueueType>()] = {
		getVkQueue(QueueType::Graphic),
		getVkQueue(QueueType::Compute),
		getVkQueue(QueueType::Copy)
	};

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

	VkCommandBuffer cmds[EnumCount<QueueType>()] = {
		vk_frame.mainCommandLists[EnumToIndex(QueueType::Graphic)].vk_command,
		vk_frame.mainCommandLists[EnumToIndex(QueueType::Compute)].vk_command,
		vk_frame.mainCommandLists[EnumToIndex(QueueType::Copy)].vk_command,
	};
	static const char* s_queueName[EnumCount<QueueType>()] = {
		"Graphic queue",
		"Compute queue",
		"Copy queue",
	};
	for (QueueType queue : EnumRange<QueueType>())
	{
		uint32_t i = EnumToIndex(queue);
		VkSemaphore signalSemaphore = vk_frame.presentSemaphore[i];
		// Async compute & transfer do not need to wait for swapchain
		Vector<VkSemaphore> waitSemaphores;
		if (queue == QueueType::Graphic)
			waitSemaphores.append(vk_frame.acquireSemaphore);
		VkFence fence = vk_frame.presentFence[i];
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmds[i];
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		submitInfo.pWaitSemaphores = waitSemaphores.data();

		color4f markerColor(0.8f, 0.8f, 0.8f, 1.f);
		beginMarker(queue, s_queueName[i], markerColor.data);
		VK_CHECK_RESULT(vkQueueSubmit(vk_queues[i], 1, &submitInfo, fence));
		endMarker(queue);
	}

	// Present
	SwapchainStatus status = m_swapchain.present(this, vk_frame);

#ifdef ENABLE_RENDERDOC_CAPTURE
	// Capture frame
	// https://renderdoc.org/docs/in_application_api.html
	if (m_renderDocContext && m_captureState == RenderDocCaptureState::Capturing)
	{
		AKA_ASSERT(m_renderDocContext->IsFrameCapturing() == 1, "Frame not capturing...");
		bool captureSucceeded = m_renderDocContext->EndFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(getVkInstance()), m_context.getPlatform()->getNativeHandle());
		uint32_t captureCount = m_renderDocContext->GetNumCaptures();
		if (captureSucceeded)
		{
			m_captureState = RenderDocCaptureState::Idle;
			if (captureCount > 0)
			{
				uint32_t lastCaptureID = captureCount - 1;
				bool inRange = m_renderDocContext->GetCapture(lastCaptureID, NULL, NULL, NULL);
				if (inRange) // if capture is valid
				{
					// With 1 as first arg, renderdoc will connect to process immediately, 
					// so call ShowReplayUI after that.
					if (m_renderDocContext->IsTargetControlConnected())
					{
						m_renderDocContext->ShowReplayUI();
					}
					else
					{
						// Zero if failed.
						uint32_t processPID = m_renderDocContext->LaunchReplayUI(1, "");
					}
				}
				else
				{
					Logger::error("Failed to retrieve renderdoc capture.");
				}
			}
			else
			{
				AlertModal(AlertModalType::Warning, "Failed connection", "The frame capture failed, try closing the app if its open.");
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

void VulkanGraphicDevice::beginMarker(QueueType queue, const char* name, const float* color)
{
	VkDebugUtilsLabelEXT label{};
	label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	memcpy(label.color, color, sizeof(float) * 4);
	label.pLabelName = name;
	vkQueueBeginDebugUtilsLabelEXT(getVkQueue(queue), &label);
}

void VulkanGraphicDevice::endMarker(QueueType queue)
{
	vkQueueEndDebugUtilsLabelEXT(getVkQueue(queue));
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
#else
	Logger::warn("Capture disabled");
#endif
}

};
};

#endif
