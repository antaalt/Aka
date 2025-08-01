#include "VulkanGraphicDevice.h"

#include <Aka/Memory/Allocator.h>
#include <Aka/OS/OS.h>

#if defined(AKA_USE_VULKAN)

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <renderdoc_app.h>

#include "VulkanCommon.hpp"
#include "VulkanInstance.h"

namespace aka {
namespace gfx {

VulkanGraphicDevice::VulkanGraphicDevice(VulkanInstance* instance, VulkanSurface* surface, VkPhysicalDevice _device, PhysicalDeviceFeatures features, const PhysicalDeviceLimits& limits) :
	m_instance(instance),
	m_surface(surface),
	m_device(VK_NULL_HANDLE),
	m_physicalDevice(_device),
	m_physicalDeviceFeatures(features),
	m_physicalDeviceLimits(limits),
	m_commandPool{ VK_NULL_HANDLE }
{
}

VulkanGraphicDevice::~VulkanGraphicDevice()
{
}

VkCommandPool createCommandPool(VkDevice device, uint32_t queueIndex)
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queueIndex;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkCommandPool commandPool;
	vkCreateCommandPool(device, &createInfo, getVkAllocator(), &commandPool);
	return commandPool;
}
VkDevice VulkanGraphicDevice::createLogicalDevice(const char* const* deviceExtensions, size_t deviceExtensionCount)
{
	VkSurfaceKHR vk_surface = m_surface->vk_surface;
	const bool hasSurface = vk_surface != VK_NULL_HANDLE;
	// --- Get device family queue
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

	Vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

	// Select graphic & present queue.
	// With exclusive swapchain, we need to use same queue to avoid transferring ownership.
	// https://stackoverflow.com/questions/55272626/what-is-actually-a-queue-family-in-vulkan/55273688#55273688
	Vector<uint32_t> queueFamilySlotCount(queueFamilyCount, 0);
	for (uint32_t iQueue = 0; iQueue < queueFamilyCount; ++iQueue)
	{
		// Should always have at least 1 queue
		const VkQueueFamilyProperties& queueFamily = queueFamilies[iQueue];
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			if (hasSurface)
			{
				VkBool32 presentSupport = VK_FALSE;
				VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, iQueue, vk_surface, &presentSupport));
				if (presentSupport == VK_TRUE)
				{
					m_presentQueue.familyIndex = iQueue;
					m_presentQueue.index = queueFamilySlotCount[iQueue]++;
					m_queues[EnumToIndex(QueueType::Graphic)].familyIndex = m_presentQueue.familyIndex;
					m_queues[EnumToIndex(QueueType::Graphic)].index = m_presentQueue.index;
					break;
				}
			}
			else
			{
				m_queues[EnumToIndex(QueueType::Graphic)].familyIndex = iQueue;
				m_queues[EnumToIndex(QueueType::Graphic)].index = queueFamilySlotCount[iQueue]++;
				break;
			}
		}
	}
	// Now we can select a queue for asyncCopy & asyncCompute
	bool isQueuePicked[EnumCount<QueueType>()] = { false };
	for (uint32_t iQueue = 0; iQueue < queueFamilyCount; ++iQueue)
	{
		const VkQueueFamilyProperties& queueFamily = queueFamilies[iQueue];
		const bool isGraphicQueue = (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT);
		const bool isComputeQueue = (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT);
		const bool isCopyQueue = (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT);
		if (!isQueuePicked[EnumToIndex(QueueType::Compute)] && isComputeQueue && !isGraphicQueue && (queueFamilySlotCount[iQueue] < queueFamily.queueCount))
		{
			isQueuePicked[EnumToIndex(QueueType::Compute)] = true;
			m_queues[EnumToIndex(QueueType::Compute)].familyIndex = iQueue;
			m_queues[EnumToIndex(QueueType::Compute)].index = queueFamilySlotCount[iQueue]++;
			AKA_ASSERT(queueFamilySlotCount[iQueue] <= queueFamily.queueCount, "Too many queues");
		}
		if (!isQueuePicked[EnumToIndex(QueueType::Copy)] && isCopyQueue && !isGraphicQueue && (queueFamilySlotCount[iQueue] < queueFamily.queueCount))
		{
			isQueuePicked[EnumToIndex(QueueType::Copy)] = true;
			m_queues[EnumToIndex(QueueType::Copy)].familyIndex = iQueue;
			m_queues[EnumToIndex(QueueType::Copy)].index = queueFamilySlotCount[iQueue]++;
			AKA_ASSERT(queueFamilySlotCount[iQueue] <= queueFamily.queueCount, "Too many queues");
		}
	}

	// Check standard queues
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		if (m_queues[i].familyIndex == VulkanQueue::invalidFamilyIndex)
		{
			Logger::error("No valid queue found for queue ", i);
			return VK_NULL_HANDLE;
		}
	}
	// Check present queue
	if (hasSurface && m_presentQueue.familyIndex == VulkanQueue::invalidFamilyIndex)
	{
		Logger::error("No valid present queue found.");
		return VK_NULL_HANDLE;
	}

	// Queues
	HashSet<uint32_t> uniqueQueueFamilies;
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
		uniqueQueueFamilies.insert(m_queues[i].familyIndex);
	if (hasSurface)
		uniqueQueueFamilies.insert(m_presentQueue.familyIndex);

	Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	HashMap<uint32_t, Vector<float>> priorities;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		// Only handle separated queue here, ignore present shared with graphics.
		for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
			if (m_queues[i].familyIndex == queueFamily)
				priorities[queueFamily].append(1.f); // TODO More prio for graphic ?
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = (uint32_t)priorities[queueFamily].size();
		queueCreateInfo.pQueuePriorities = priorities[queueFamily].data();
		queueCreateInfos.append(queueCreateInfo);
	}

	// --- Create device
	void* next = nullptr;
	// VK_VERSION_1_2
	VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
	if (asBool(m_physicalDeviceFeatures & PhysicalDeviceFeatures::BindlessResources))
	{
		indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		indexingFeatures.pNext = nullptr;
		indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
		indexingFeatures.runtimeDescriptorArray = VK_TRUE;
		indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
		next = &indexingFeatures;
	}

	// VK_VERSION_1_2
	VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures{};
	timelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
	timelineSemaphoreFeatures.pNext = next;
	timelineSemaphoreFeatures.timelineSemaphore = VK_TRUE;
	next = &timelineSemaphoreFeatures;

	// VK_VERSION_1_1
	VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
	if (asBool(m_physicalDeviceFeatures & PhysicalDeviceFeatures::MeshShader))
	{
		meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
		meshShaderFeatures.pNext = &timelineSemaphoreFeatures;
		meshShaderFeatures.taskShader = VK_TRUE;
		meshShaderFeatures.meshShader = VK_TRUE;
		next = &meshShaderFeatures;
	}

	// VK_VERSION_1_0
	VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR fragmentShaderBarycentricsFeature{};
	if (asBool(m_physicalDeviceFeatures & PhysicalDeviceFeatures::Barycentric))
	{
		fragmentShaderBarycentricsFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR;
		fragmentShaderBarycentricsFeature.pNext = next;
		fragmentShaderBarycentricsFeature.fragmentShaderBarycentric = VK_TRUE;
		next = &fragmentShaderBarycentricsFeature;
	}

	// VK_VERSION_1_1
	VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomicFloatFeatures{};
	if (asBool(m_physicalDeviceFeatures & PhysicalDeviceFeatures::AtomicFloat))
	{
		atomicFloatFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT;
		atomicFloatFeatures.pNext = next;
		atomicFloatFeatures.shaderBufferFloat32AtomicAdd = VK_TRUE;
		atomicFloatFeatures.shaderBufferFloat32Atomics = VK_TRUE;
		next = &atomicFloatFeatures;
	}

	// VK_VERSION_1_1
	VkPhysicalDeviceFeatures2 deviceFeatures{};
	deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures.pNext = next;
	deviceFeatures.features.samplerAnisotropy = VK_TRUE;
	deviceFeatures.features.fragmentStoresAndAtomics = VK_TRUE;
	deviceFeatures.features.shaderFloat64 = VK_TRUE;
	deviceFeatures.features.multiDrawIndirect = VK_TRUE;
	deviceFeatures.features.fillModeNonSolid = VK_TRUE; // VK_POLYGON_MODE_LINE

	// Renderdoc seems to require these features:
	deviceFeatures.features.geometryShader = VK_TRUE; // render doc
	deviceFeatures.features.sampleRateShading = VK_TRUE; // render doc

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = &deviceFeatures;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = NULL; // VK_VERSION_1_0
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionCount);
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	if (m_instance->isValidationLayerEnabled())
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_instance->getValidationLayerCount());
		createInfo.ppEnabledLayerNames = m_instance->getValidationLayers();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}
	VkDevice device = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateDevice(m_physicalDevice, &createInfo, getVkAllocator(), &device));

	// Retrieve queues from device
	static const char* s_queueName[EnumCount<QueueType>()] = {
		"Graphic queue",
		"Compute queue",
		"Copy queue",
	};
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		vkGetDeviceQueue(device, m_queues[i].familyIndex, m_queues[i].index, &m_queues[i].queue);
		setDebugName(device, m_queues[i].queue, "%s", s_queueName[i]);
	}
	if (hasSurface)
	{
		vkGetDeviceQueue(device, m_presentQueue.familyIndex, m_presentQueue.index, &m_presentQueue.queue);
		setDebugName(device, m_presentQueue.queue, "Present queue");
	}
	return device;
}

void VulkanGraphicDevice::initialize()
{
	// Create logical device.
	Vector<const char*> usedDeviceExtensions;
	for (uint32_t i = 0; i < m_instance->getRequiredDeviceExtensionCount(); i++)
	{
		usedDeviceExtensions.append(m_instance->getRequiredDeviceExtensions()[i]);
	}
	if (asBool(m_physicalDeviceFeatures & PhysicalDeviceFeatures::MeshShader))
	{
		usedDeviceExtensions.append(VK_EXT_MESH_SHADER_EXTENSION_NAME);
	}
	if (asBool(m_physicalDeviceFeatures & PhysicalDeviceFeatures::AtomicFloat))
	{
		usedDeviceExtensions.append(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
	}
	if (asBool(m_physicalDeviceFeatures & PhysicalDeviceFeatures::BindlessResources))
	{
		usedDeviceExtensions.append(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
	}
	if (asBool(m_physicalDeviceFeatures & PhysicalDeviceFeatures::Barycentric))
	{
		usedDeviceExtensions.append(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME);
	}
	m_device = createLogicalDevice(usedDeviceExtensions.data(), usedDeviceExtensions.size());

	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		m_commandPool[i] = createCommandPool(m_device, m_queues[i].familyIndex);
		setDebugName(m_device, m_commandPool[i], "MainCommandPool", i);
	}

	// Create shared staging memory for improved upload
	m_stagingUploadBuffer = VulkanBuffer::createVkBuffer(getVkDevice(), m_stagingUploadHeapSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	m_stagingUploadMemory = VulkanBuffer::createVkDeviceMemory(getVkDevice(), getVkPhysicalDevice(), m_stagingUploadBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	// Create shared staging memory for improved download
	m_stagingDownloadBuffer = VulkanBuffer::createVkBuffer(getVkDevice(), m_stagingDownloadHeapSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	m_stagingDownloadMemory = VulkanBuffer::createVkDeviceMemory(getVkDevice(), getVkPhysicalDevice(), m_stagingDownloadBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	// Create copy fence 
	m_copyFenceCounter = 0;
	m_copyFenceHandle = createFence("CopyFence", m_copyFenceCounter);
}

void VulkanGraphicDevice::shutdown()
{
	wait();

	destroy(m_copyFenceHandle);

	vkFreeMemory(getVkDevice(), m_stagingUploadMemory, getVkAllocator());
	vkDestroyBuffer(getVkDevice(), m_stagingUploadBuffer, getVkAllocator());

	vkFreeMemory(getVkDevice(), m_stagingDownloadMemory, getVkAllocator());
	vkDestroyBuffer(getVkDevice(), m_stagingDownloadBuffer, getVkAllocator());

	// Release all resources before destroying context.
	// We still check if resources where cleanly destroyed before releasing the remains.
	// TODO check recursive release issue
	m_commandEncoderPool.release([this](VulkanCommandEncoder& res) { Logger::warn("Leaking command list."); this->release(&res); });
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
	m_swapchainPool.release([this](const VulkanSwapchain& res) { Logger::warn("Leaking swapchain ", res.name); this->destroy(SwapchainHandle{ &res }); });
	m_renderPassPool.release([this](const VulkanRenderPass& res) { Logger::warn("Leaking render pass ", res.name); this->destroy(RenderPassHandle{ &res }); });

	for (auto& rp : m_descriptorSetLayouts)
	{
		vkDestroyDescriptorSetLayout(m_device, rp.second, getVkAllocator());
		//vkDestroyPipelineLayout(device, rp.second.pipelineLayout, nullptr);
	}
	for (auto& rp : m_pipelineLayout)
	{
		vkDestroyPipelineLayout(m_device, rp.second, getVkAllocator());
	}
	for (auto& rp : m_renderPassState)
	{
		vkDestroyRenderPass(m_device, rp.second, getVkAllocator());
	}
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
		vkDestroyCommandPool(m_device, m_commandPool[i], getVkAllocator());
	vkDestroyDevice(m_device, getVkAllocator());
}

GraphicAPI VulkanGraphicDevice::getApi() const
{
	return GraphicAPI::Vulkan;
}
PhysicalDeviceFeatures VulkanGraphicDevice::getFeatures() const
{
	return m_physicalDeviceFeatures;
}
bool VulkanGraphicDevice::hasFeatures(PhysicalDeviceFeatures _features) const
{
	return asBool(m_physicalDeviceFeatures & _features);
}
PhysicalDeviceLimits VulkanGraphicDevice::getLimits() const
{
	return m_physicalDeviceLimits;
}

CommandEncoder* VulkanGraphicDevice::acquireCommandEncoder(FrameHandle frame, QueueType queue)
{
	VulkanFrame* vk_frame = getVk<VulkanFrame>(frame);
	return vk_frame->allocateCommand(this, queue);
}

void VulkanGraphicDevice::release(FrameHandle frame, CommandEncoder* cmd)
{
	VulkanFrame* vk_frame = getVk<VulkanFrame>(frame);
	return vk_frame->releaseCommand(reinterpret_cast<VulkanCommandEncoder*>(cmd));
}

const Frame* VulkanGraphicDevice::get(FrameHandle handle)
{
	return static_cast<const Frame*>(handle.__data);
}
FrameHandle VulkanGraphicDevice::frame(SwapchainHandle handle)
{
	VulkanSwapchain* vk_swapchain = getVk<VulkanSwapchain>(handle);
#ifdef ENABLE_RENDERDOC_CAPTURE
	RENDERDOC_API_1_6_0* context = m_instance->getRenderDocContext();
	if (context && m_instance->getRenderDocCaptureState() == RenderDocCaptureState::PendingCapture)
	{
		m_instance->setRenderDocCaptureState(RenderDocCaptureState::Capturing);
		// Match all window by passing NULL
		VkInstance instance = m_instance->getVkInstance();
		context->StartFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(instance), NULL);
		AKA_ASSERT(context->IsFrameCapturing() == 1, "Frame not capturing...");
	}
#endif
	VulkanFrame* vk_frame = vk_swapchain->acquireNextImage(this);
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
		CommandList* cmd = vk_frame->mainCommandEncoders[i]->begin();
		cmd->beginMarker(s_commandName[i], markerColor.data);
	}
	return FrameHandle{ vk_frame };
}

SwapchainStatus VulkanGraphicDevice::present(SwapchainHandle handle, FrameHandle frame)
{
	VulkanSwapchain* vk_swapchain = getVk<VulkanSwapchain>(handle);
	if (m_commandEncoderToRelease.size() > 0)
	{
		// Wait for queue to complete before presenting.
		// Not optimal but we need some way to destroy them & ensure job is done before submitting main work.
		// How to handle async queue then ?
		// Copy queue can be expected to finish before frame start though.
		wait(m_copyFenceHandle, m_copyFenceCounter);
		// Should reset them though...
		//m_copyFenceCounter = 0; // Reset
		for (VulkanCommandEncoder* encoder : m_commandEncoderToRelease)
		{
			release(encoder);
		}
		m_commandEncoderToRelease.clear();
	}
	VulkanFrame& vk_frame = vk_swapchain->getVkFrame(frame);

	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		VulkanCommandEncoder* encoder = vk_frame.mainCommandEncoders[i];
		VulkanCommandList& cmd = encoder->getCommandList();
		cmd.endMarker();
		encoder->end(&cmd);
	}

	// Submit
	VkQueue vk_queues[EnumCount<QueueType>()] = {
		getVkQueue(QueueType::Graphic),
		getVkQueue(QueueType::Compute),
		getVkQueue(QueueType::Copy)
	};

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

	VkCommandBuffer cmds[EnumCount<QueueType>()] = {
		vk_frame.mainCommandEncoders[EnumToIndex(QueueType::Graphic)]->getVkCommandBuffer(),
		vk_frame.mainCommandEncoders[EnumToIndex(QueueType::Compute)]->getVkCommandBuffer(),
		vk_frame.mainCommandEncoders[EnumToIndex(QueueType::Copy)]->getVkCommandBuffer(),
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
	SwapchainStatus status = vk_swapchain->present(this, vk_frame);

#ifdef ENABLE_RENDERDOC_CAPTURE
	// Capture frame
	// https://renderdoc.org/docs/in_application_api.html
	RENDERDOC_API_1_6_0* context = m_instance->getRenderDocContext();
	if (context && m_instance->getRenderDocCaptureState() == RenderDocCaptureState::Capturing)
	{
		AKA_ASSERT(context->IsFrameCapturing() == 1, "Frame not capturing...");
		bool captureSucceeded = context->EndFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(m_instance->getVkInstance()), NULL);
		uint32_t captureCount = context->GetNumCaptures();
		if (captureSucceeded)
		{
			m_instance->setRenderDocCaptureState(RenderDocCaptureState::Idle);
			if (captureCount > 0)
			{
				uint32_t lastCaptureID = captureCount - 1;
				bool inRange = context->GetCapture(lastCaptureID, NULL, NULL, NULL);
				if (inRange) // if capture is valid
				{
					// With 1 as first arg, renderdoc will connect to process immediately, 
					// so call ShowReplayUI after that.
					if (context->IsTargetControlConnected())
					{
						context->ShowReplayUI();
					}
					else
					{
						// Zero if failed.
						uint32_t processPID = context->LaunchReplayUI(1, "");
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
			m_instance->setRenderDocCaptureState(RenderDocCaptureState::Idle);
			Logger::error("Renderdoc capture failed.");
		}
	}
#endif
	return status;
}
VkInstance VulkanGraphicDevice::getVkInstance() 
{ 
	return m_instance->getVkInstance();
}

VkRenderPass VulkanGraphicDevice::getVkRenderPass(const RenderPassState& state)
{
	auto it = m_renderPassState.find(state);
	if (it != m_renderPassState.end())
		return it->second;
	VkRenderPass vk_renderPass = VulkanRenderPass::createVkRenderPass(m_device, state);
	m_renderPassState.insert(std::make_pair(state, vk_renderPass));
	size_t hash = std::hash<RenderPassState>()(state);
	setDebugName(m_device, vk_renderPass, "RenderPassFramebuffer", hash);
	return vk_renderPass;
}

VkDescriptorSetLayout VulkanGraphicDevice::getVkDescriptorSetLayout(const ShaderBindingState& bindingsDesc)
{
	if (bindingsDesc.count == 0 || bindingsDesc.count > ShaderMaxBindingCount)
		return VK_NULL_HANDLE;
	auto it = m_descriptorSetLayouts.find(bindingsDesc);
	if (it != m_descriptorSetLayouts.end())
		return it->second;
	VkDescriptorSetLayout layout = VulkanDescriptorSet::createVkDescriptorSetLayout(m_device, bindingsDesc);

	size_t hash = std::hash<ShaderBindingState>()(bindingsDesc);
	setDebugName(m_device, layout, "VkDescriptorSetLayout_", hash);

	m_descriptorSetLayouts.insert(std::make_pair(bindingsDesc, layout));
	return layout;
}

VkPipelineLayout VulkanGraphicDevice::getVkPipelineLayout(const VkDescriptorSetLayout* layouts, uint32_t layoutCount, const VkPushConstantRange* constants, uint32_t constantCount)
{
	PipelineLayoutKey pair = std::make_pair(
		Vector<VkDescriptorSetLayout>(layouts, layoutCount),
		Vector<VkPushConstantRange>(constants, constantCount)
	);
	auto it = m_pipelineLayout.find(pair);
	if (it != m_pipelineLayout.end())
		return it->second;

	// Create the layout of the pipeline following the provided descriptor set layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = layoutCount;
	pipelineLayoutCreateInfo.pSetLayouts = layouts;
	pipelineLayoutCreateInfo.pushConstantRangeCount = constantCount;
	pipelineLayoutCreateInfo.pPushConstantRanges = constants;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, getVkAllocator(), &pipelineLayout));

	m_pipelineLayout.insert(std::make_pair(pair, pipelineLayout));

	return pipelineLayout;
}
uint32_t VulkanGraphicDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	return VulkanGraphicDevice::findMemoryType(m_physicalDevice, typeFilter, properties);
}

uint32_t VulkanGraphicDevice::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	// Should check if device local if VkPhysicalDeviceMemoryProperties::memoryHeaps has VK_MEMORY_HEAP_DEVICE_LOCAL_BIT
	AKA_ASSERT(false, "Failed to find suitable memory type!");
	return 0;
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
	VK_CHECK_RESULT(vkDeviceWaitIdle(m_device));
}

void VulkanGraphicDevice::screenshot(void* data)
{
	AKA_NOT_IMPLEMENTED;
}

void VulkanGraphicDevice::capture()
{
#ifdef ENABLE_RENDERDOC_CAPTURE
	RENDERDOC_API_1_6_0* context = m_instance->getRenderDocContext();
	if (context)
	{
		m_instance->setRenderDocCaptureState(RenderDocCaptureState::PendingCapture);
	}
#else
	Logger::warn("Capture disabled");
#endif
}

};
};

#endif
