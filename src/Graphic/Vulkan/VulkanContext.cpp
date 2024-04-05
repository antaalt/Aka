#include "VulkanContext.h"

#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanFrameBuffer.h"
#include "VulkanProgram.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "Platform/GLFW3/PlatformGLFW3.h"

#include <Aka/Core/Container/String.h>

#include <set>

#if defined(AKA_USE_VULKAN)

namespace aka {
namespace gfx {

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData
)
{
	String message = "[vulkan]";
	switch (messageType)
	{
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		message += "[general]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		message += "[validation]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		message += "[performance]";
		break;
	default:
		message += "[unknown]";
		break;
	}
	switch (messageSeverity)
	{
	default:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		Logger::debug(message, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		Logger::info(message, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		Logger::warn(message, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		Logger::error(message, pCallbackData->pMessage);
		AKA_DEBUG_BREAK;
		break;
	}
	return VK_FALSE;
}

#if defined(AKA_DEBUG)
static constexpr const bool enableValidationLayers = true;
#else
static constexpr const bool enableValidationLayers = false;
#endif
static constexpr const char* validationLayers[] = {
	"VK_LAYER_KHRONOS_validation"
};
static constexpr const size_t validationLayerCount = sizeof(validationLayers) / sizeof(*validationLayers);

bool hasValidationLayerSupport(const char*const* validationLayers, size_t count)
{
	uint32_t availableLayerCount;
	VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr));

	std::vector<VkLayerProperties> availableLayers(availableLayerCount);
	VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data()));

	for (size_t i = 0; i < count; i++)
	{
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (String::compare(validationLayers[i], layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
			return false;
	}
	return true;
}

void populateCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo)
{
	debugCreateInfo = {};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback = debugCallback;
	debugCreateInfo.pUserData = nullptr; // Optional
}

VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance)
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	populateCreateInfo(createInfo);
	VkDebugUtilsMessengerEXT messenger;
	VK_CHECK_RESULT(vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &messenger));
	return messenger;
}

VkRenderPass VulkanContext::getRenderPass(const RenderPassState& state)
{
	auto it = m_renderPassState.find(state);
	if (it != m_renderPassState.end())
		return it->second;
	VkRenderPass vk_renderPass = VulkanRenderPass::createVkRenderPass(device, state);
	m_renderPassState.insert(std::make_pair(state, vk_renderPass));
	size_t hash = std::hash<RenderPassState>()(state);
	setDebugName(device, vk_renderPass, "RenderPassFramebuffer", hash);
	return vk_renderPass;
}

VkDescriptorSetLayout VulkanContext::getDescriptorSetLayout(const ShaderBindingState& bindingsDesc)
{
	if (bindingsDesc.count == 0 || bindingsDesc.count > ShaderMaxBindingCount)
		return VK_NULL_HANDLE;
	auto it = m_descriptorSetLayouts.find(bindingsDesc);
	if (it != m_descriptorSetLayouts.end())
		return it->second;
	VkDescriptorSetLayout layout = VulkanDescriptorSet::createVkDescriptorSetLayout(device, bindingsDesc);

	size_t hash = std::hash<ShaderBindingState>()(bindingsDesc);
	setDebugName(device, layout, "VkDescriptorSetLayout_", hash);

	m_descriptorSetLayouts.insert(std::make_pair(bindingsDesc, layout));
	return layout;
}

VkPipelineLayout VulkanContext::getPipelineLayout(const VkDescriptorSetLayout* layouts, uint32_t layoutCount, const VkPushConstantRange* constants, uint32_t constantCount)
{
	PipelineLayoutKey pair = std::make_pair(
		std::vector<VkDescriptorSetLayout>(layouts, layouts + layoutCount),
		std::vector<VkPushConstantRange>(constants, constants + constantCount)
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
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	m_pipelineLayout.insert(std::make_pair(pair, pipelineLayout));

	return pipelineLayout;
}

VkInstance VulkanContext::createInstance(const char** instanceExtensions, size_t instanceExtensionCount)
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	for (size_t i = 0; i < instanceExtensionCount; i++)
	{
		bool found = false;
		for (const VkExtensionProperties& extension : extensions)
		{
			if (String::compare(extension.extensionName, instanceExtensions[i]) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			Logger::error("Instance extension '", instanceExtensions[i], "' not available.");
			return VK_NULL_HANDLE;
		}
	}
	// Create instance
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Aka";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Aka";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensionCount);
	createInfo.ppEnabledExtensionNames = instanceExtensions;
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers)
	{

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayerCount);
		createInfo.ppEnabledLayerNames = validationLayers;
		populateCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}
	VkInstance instance;
	VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));
	return instance;
}

VkSurfaceKHR VulkanContext::createSurface(PlatformDevice* platform)
{
	VkSurfaceKHR surface;
	VK_CHECK_RESULT(glfwCreateWindowSurface(
		instance,
		reinterpret_cast<PlatformGLFW3*>(platform)->getGLFW3Handle(),
		nullptr,
		&surface
	));
	return surface;
}

const char** VulkanContext::getPlatformRequiredInstanceExtension(const PlatformDevice* platform, uint32_t* count)
{
	// TODO check glfw
	return glfwGetRequiredInstanceExtensions(count);
}

VkPhysicalDevice VulkanContext::pickPhysicalDevice(PickPhysicalDeviceFunc isPhysicalDeviceSuitable)
{
	uint32_t deviceCount = 0;
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
	if (deviceCount == 0)
	{
		Logger::error("No physical device found.");
		return VK_NULL_HANDLE;
	}
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()));
	for (const VkPhysicalDevice& physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceProperties deviceProperties{};
		VkPhysicalDeviceFeatures2 deviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);
		if (isPhysicalDeviceSuitable(deviceProperties, deviceFeatures))
			return physicalDevice;
	}
	Logger::error("No suitable physical device found.");
	return VK_NULL_HANDLE;
}

VkDevice VulkanContext::createLogicalDevice(const char** deviceExtensions, size_t deviceExtensionCount)
{
	const bool hasSurface = true;
	// --- Get device family queue
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	Vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

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
				VkBool32 presentSupport = false;
				VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, iQueue, surface, &presentSupport));
				if (presentSupport)
				{
					presentQueue.familyIndex = iQueue;
					presentQueue.index = queueFamilySlotCount[iQueue]++;
					queues[EnumToIndex(QueueType::Graphic)].familyIndex = presentQueue.familyIndex;
					queues[EnumToIndex(QueueType::Graphic)].index = presentQueue.index;
					break;
				}
			}
			else
			{
				queues[EnumToIndex(QueueType::Graphic)].familyIndex = iQueue;
				queues[EnumToIndex(QueueType::Graphic)].index = queueFamilySlotCount[iQueue]++;
				break;
			}
		}
	}
	// Now we can select a queue for asyncCopy & asyncCompute
	bool isQueuePicked[EnumCount<QueueType>()] = { false };
	for (uint32_t iQueue = 0; iQueue < queueFamilyCount; ++iQueue)
	{
		const VkQueueFamilyProperties& queueFamily = queueFamilies[iQueue];
		if (!isQueuePicked[EnumToIndex(QueueType::Compute)] && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) && (queueFamilySlotCount[iQueue] < queueFamily.queueCount))
		{
			isQueuePicked[EnumToIndex(QueueType::Compute)] = true;
			queues[EnumToIndex(QueueType::Compute)].familyIndex = iQueue;
			queues[EnumToIndex(QueueType::Compute)].index = queueFamilySlotCount[iQueue]++;
			AKA_ASSERT(queueFamilySlotCount[iQueue] <= queueFamily.queueCount, "Too many queues");
		}
		if (!isQueuePicked[EnumToIndex(QueueType::Copy)] && (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && (queueFamilySlotCount[iQueue] < queueFamily.queueCount))
		{
			isQueuePicked[EnumToIndex(QueueType::Copy)] = true;
			queues[EnumToIndex(QueueType::Copy)].familyIndex = iQueue;
			queues[EnumToIndex(QueueType::Copy)].index = queueFamilySlotCount[iQueue]++;
			AKA_ASSERT(queueFamilySlotCount[iQueue] <= queueFamily.queueCount, "Too many queues");
		}
	}
	
	// Check standard queues
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		if (queues[i].familyIndex == VulkanQueue::invalidFamilyIndex)
		{
			Logger::error("No valid queue found for queue ", i);
			return VK_NULL_HANDLE;
		}
	}
	// Check present queue
	if (hasSurface && presentQueue.familyIndex == VulkanQueue::invalidFamilyIndex)
	{
		Logger::error("No valid present queue found.");
		return VK_NULL_HANDLE;
	}

	// Queues
	std::set<uint32_t> uniqueQueueFamilies;
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
		uniqueQueueFamilies.insert(queues[i].familyIndex);
	if (hasSurface)
		uniqueQueueFamilies.insert(presentQueue.familyIndex);
	Logger::info("Using ", uniqueQueueFamilies.size(), " queues for ", static_cast<uint32_t>(EnumCount<QueueType>()), " queue types.");

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::map<uint32_t, std::vector<float>> priorities;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		// Only handle separated queue here, ignore present shared with graphics.
		for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
			if (queues[i].familyIndex == queueFamily)
				priorities[queueFamily].push_back(1.f); // TODO More prio for graphic ?
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = (uint32_t)priorities[queueFamily].size();
		queueCreateInfo.pQueuePriorities = priorities[queueFamily].data();
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// --- Create device
	// Check available device extensions
	uint32_t extensionCount = 0;
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr));
	std::vector<VkExtensionProperties> availableDeviceExtensions(extensionCount);
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableDeviceExtensions.data()));
	for (size_t i = 0; i < deviceExtensionCount; i++)
	{
		bool foundExtension = false;
		for (const VkExtensionProperties& extension : availableDeviceExtensions)
		{
			if (String::compare(extension.extensionName, deviceExtensions[i]) == 0)
			{
				foundExtension = true;
				break;
			}
		}
		if (!foundExtension)
		{
			Logger::error("Device extension '", deviceExtensions[i], "' not available.");
			return VK_NULL_HANDLE;
		}
	}
	// VK_VERSION_1_2
	VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
	indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	indexingFeatures.pNext = nullptr;
	indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
	indexingFeatures.runtimeDescriptorArray = VK_TRUE;
	indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;

	// VK_VERSION_1_2
	VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures{};
	timelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
	timelineSemaphoreFeatures.pNext = &indexingFeatures;
	timelineSemaphoreFeatures.timelineSemaphore = VK_TRUE;

	// VK_VERSION_1_1
	VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
	meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
	meshShaderFeatures.pNext = &timelineSemaphoreFeatures;
	meshShaderFeatures.taskShader = VK_TRUE;
	meshShaderFeatures.meshShader = VK_TRUE;

	// VK_VERSION_1_1
	// TODO: Check physical device suitable for these features.
	VkPhysicalDeviceFeatures2 deviceFeatures {};
	deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures.pNext = &meshShaderFeatures;
	deviceFeatures.features.samplerAnisotropy = VK_TRUE;
	deviceFeatures.features.fragmentStoresAndAtomics = VK_TRUE;
	deviceFeatures.features.shaderFloat64 = VK_TRUE;
	deviceFeatures.features.multiDrawIndirect = VK_TRUE;
	deviceFeatures.features.fillModeNonSolid = VK_TRUE; // VK_POLYGON_MODE_LINE

	// Renderdoc seems to require these features:
	deviceFeatures.features.geometryShader = VK_TRUE; // render doc
	deviceFeatures.features.sampleRateShading = VK_TRUE; // render doc

	VkDeviceCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = &deviceFeatures;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = NULL; // VK_VERSION_1_0
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionCount);
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayerCount);
		createInfo.ppEnabledLayerNames = validationLayers;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}
	VkDevice device;
	VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &createInfo, NULL, &device));

	// Retrieve queues from device
	static const char* s_queueName[EnumCount<QueueType>()] = {
		"Graphic queue",
		"Compute queue",
		"Copy queue",
	};
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		vkGetDeviceQueue(device, queues[i].familyIndex, queues[i].index, &queues[i].queue);
		setDebugName(device, queues[i].queue, "%s", s_queueName[i]);
	}
	if (hasSurface)
	{
		vkGetDeviceQueue(device, presentQueue.familyIndex, presentQueue.index, &presentQueue.queue);
		setDebugName(device, presentQueue.queue, "Present queue");
	}
	return device;
}

VkCommandPool createCommandPool(VkDevice device, uint32_t queueIndex)
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queueIndex;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkCommandPool commandPool;
	vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);
	return commandPool;
}


void VulkanContext::initialize(PlatformDevice* platform, const GraphicConfig& config)
{
	m_platform = platform;
	// We are using Vulkan 1.2 in this backend.
	/*m_settings.api = GraphicAPI::Vulkan;
	m_settings.version.major = 1;
	m_settings.version.minor = 2;
	m_settings.profile = 450;
	m_settings.coordinates.clipSpacePositive = true; // VK clip space is [0, 1]
	m_settings.coordinates.originTextureBottomLeft = false; // VK start reading texture at top left.
	m_settings.coordinates.originUVBottomLeft = false; // VK UV origin is top left
	m_settings.coordinates.renderAxisYUp = false; // VK render axis y is down
	*/
	// Create instance & device
	// Platform extension
	uint32_t requiredPlatformInstanceExtensionCount = 0;
	const char** requiredPlatformInstanceExtensions = getPlatformRequiredInstanceExtension(platform, &requiredPlatformInstanceExtensionCount);
	AKA_ASSERT(requiredPlatformInstanceExtensions != nullptr, "GLFW failed to initialize instance extensions.");
	// Custom extensions
	const char* requiredInstanceExtensions[] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME, // Replace old VK_EXT_debug_marker & VK_EXT_debug_report 
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
	};
	const size_t requiredInstanceExtensionCount = sizeof(requiredInstanceExtensions) / sizeof(*requiredInstanceExtensions);
	// All extensions
	const size_t instanceExtensionCount = requiredPlatformInstanceExtensionCount + requiredInstanceExtensionCount;
	Vector<const char*> instanceExtensions(instanceExtensionCount);
	for (size_t i = 0; i < requiredPlatformInstanceExtensionCount; i++)
		instanceExtensions[i] = requiredPlatformInstanceExtensions[i];
	for (size_t i = 0; i < requiredInstanceExtensionCount; i++)
		instanceExtensions[requiredPlatformInstanceExtensionCount + i] = requiredInstanceExtensions[i];
	// Device extensions
	const char* deviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
		VK_EXT_MESH_SHADER_EXTENSION_NAME,
	};
	const size_t deviceExtensionCount = sizeof(deviceExtensions) / sizeof(*deviceExtensions);
	if (!hasValidationLayerSupport(validationLayers, validationLayerCount))
	{
		Logger::warn("Validation layers unsupported");
	}
	instance = createInstance(instanceExtensions.data(), instanceExtensionCount);
	debugMessenger = createDebugMessenger(instance);
	surface = createSurface(platform);

	physicalDevice = pickPhysicalDevice([](const VkPhysicalDeviceProperties& properties, const VkPhysicalDeviceFeatures2& features) {
		bool supported = true;
		supported &= properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
		supported &= features.features.samplerAnisotropy == VK_TRUE;
		supported &= features.features.fragmentStoresAndAtomics == VK_TRUE;
		supported &= features.features.shaderFloat64 == VK_TRUE;
		supported &= features.features.multiDrawIndirect == VK_TRUE;
		return supported;
	});
	device = createLogicalDevice(deviceExtensions, deviceExtensionCount);

	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		commandPool[i] = createCommandPool(device, queues[i].familyIndex);
		setDebugName(device, commandPool[i], "MainCommandPool", i);
	}

}

void VulkanContext::shutdown()
{
	for (auto& rp : m_descriptorSetLayouts)
	{
		vkDestroyDescriptorSetLayout(device, rp.second, nullptr);
		//vkDestroyPipelineLayout(device, rp.second.pipelineLayout, nullptr);
	}
	for (auto& rp : m_pipelineLayout)
	{
		vkDestroyPipelineLayout(device, rp.second, nullptr);
	}
	for (auto& rp : m_renderPassState)
	{
		vkDestroyRenderPass(device, rp.second, nullptr);
	}
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
		vkDestroyCommandPool(device, commandPool[i], nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(device, nullptr);
	// physical device
	vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	vkDestroyInstance(instance, nullptr);
}

uint32_t VulkanContext::getPhysicalDeviceCount() const
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	return deviceCount;
}

PhysicalDevice* VulkanContext::getPhysicalDevice(uint32_t index)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	//m_features.maxTextureUnits = deviceProperties.limits.maxDescriptorSetSampledImages;
	//m_features.maxTextureSize = deviceProperties.limits.maxImageDimension2D;
	//m_features.maxColorAttachments = deviceProperties.limits.maxColorAttachments;
	//m_features.maxElementIndices = deviceProperties.limits.maxDrawIndexedIndexValue;
	//m_features.maxElementVertices = deviceProperties.limits.maxVertexInputBindings;
	return nullptr; // pool
}

uint32_t VulkanContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	return VulkanContext::findMemoryType(physicalDevice, typeFilter, properties);
}

uint32_t VulkanContext::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	// Should check if device local if VkPhysicalDeviceMemoryProperties::memoryHeaps has VK_MEMORY_HEAP_DEVICE_LOCAL_BIT
	throw std::runtime_error("failed to find suitable memory type!");
}

VkFormat VulkanContext::tovk(TextureFormat format)
{
	switch (format)
	{
	default:
		AKA_ASSERT(false, "Invalid texture format");
		return VK_FORMAT_UNDEFINED;
	case TextureFormat::R8:
	case TextureFormat::R8U:
		return VK_FORMAT_R8_UNORM; // VK_FORMAT_R8_UINT
	case TextureFormat::R16:
	case TextureFormat::R16U:
		return VK_FORMAT_R16_UNORM;
	case TextureFormat::R16F:
		return VK_FORMAT_R16_SFLOAT;
	case TextureFormat::R32F:
		return VK_FORMAT_R32_SFLOAT;
	case TextureFormat::RG8:
	case TextureFormat::RG8U:
		return VK_FORMAT_R8G8_UNORM;
	case TextureFormat::RG16:
	case TextureFormat::RG16U:
		return VK_FORMAT_R16G16_UNORM;
	case TextureFormat::RG16F:
		return VK_FORMAT_R16G16_SFLOAT;
	case TextureFormat::RG32F:
		return VK_FORMAT_R32G32_SFLOAT;
	case TextureFormat::RGB8:
	case TextureFormat::RGB8U:
		return VK_FORMAT_R8G8B8_UNORM;
	case TextureFormat::RGB16:
	case TextureFormat::RGB16U:
		return VK_FORMAT_R16G16B16_UNORM;
	case TextureFormat::RGB16F:
		return VK_FORMAT_R16G16B16_SFLOAT;
	case TextureFormat::RGB32F:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case TextureFormat::RGBA8:
	case TextureFormat::RGBA8U:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case TextureFormat::RGBA16:
	case TextureFormat::RGBA16U:
		return VK_FORMAT_R16G16B16A16_UNORM;
	case TextureFormat::RGBA16F:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case TextureFormat::RGBA32F:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case TextureFormat::BGRA:
	case TextureFormat::BGRA8:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case TextureFormat::Depth16:
		return VK_FORMAT_D16_UNORM;
	case TextureFormat::Depth24:
		return VK_FORMAT_D24_UNORM_S8_UINT;
	case TextureFormat::Depth:
	case TextureFormat::Depth32:
	case TextureFormat::Depth32F:
		return VK_FORMAT_D32_SFLOAT;
	case TextureFormat::Depth0Stencil8:
		return VK_FORMAT_S8_UINT;
	case TextureFormat::DepthStencil:
	case TextureFormat::Depth24Stencil8:
		return VK_FORMAT_D24_UNORM_S8_UINT;
	case TextureFormat::Depth32FStencil8:
		return VK_FORMAT_D32_SFLOAT_S8_UINT;
	}
	return VK_FORMAT_UNDEFINED;
}

VkFilter VulkanContext::tovk(Filter filter)
{
	switch (filter)
	{
	default:
		AKA_ASSERT(false, "Invalid filter");
		[[fallthrough]];
	case Filter::Nearest:
		return VK_FILTER_NEAREST;
	case Filter::Linear:
		return VK_FILTER_LINEAR;
	}
}

VkIndexType VulkanContext::tovk(IndexFormat format)
{
	switch (format)
	{
	case IndexFormat::UnsignedByte:
		return VK_INDEX_TYPE_UINT8_EXT;
	case IndexFormat::UnsignedShort:
		return VK_INDEX_TYPE_UINT16;
	default:
		AKA_ASSERT(false, "Invalid index format");
		[[fallthrough]];
	case IndexFormat::UnsignedInt:
		return VK_INDEX_TYPE_UINT32;
	}
}

VkBufferUsageFlagBits VulkanContext::tovk(BufferType type)
{
	switch (type)
	{
	case BufferType::Vertex:
		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	case BufferType::Index:
		return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	default:
		AKA_ASSERT(false, "Invalid buffer type");
		[[fallthrough]];
	case BufferType::Uniform:
		return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	case BufferType::Storage:
		return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	case BufferType::Indirect:
		return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	}
}

// Resource usage	| Default	| Dynamic	| Immutable	| Staging
// GPU-Read			| Yes		| Yes		| Yes		| Yes
// GPU-Write		| Yes		| --		| --		| Yes
// CPU-Read			| --		| --		| --		| Yes
// CPU-Write		| --		| Yes		| --		| Yes
VkMemoryPropertyFlags VulkanContext::tovk(BufferUsage type)
{
	// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT = can be mapped
	// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT = needed for map
	switch (type)
	{
	case BufferUsage::Default:
		return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	case BufferUsage::Dynamic: // Support map
		// Should not be only host coherent
		return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	case BufferUsage::Staging:
		return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	case BufferUsage::Immutable:
		return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	default: 
		return 0;
	}
}
//#define DEPTH_AND_STENCIL_SEPARATELY 1 // separateDepthStencilLayouts
VkImageLayout VulkanContext::tovk(ResourceAccessType type, TextureFormat format)
{
	bool depth = Texture::hasDepth(format);
	bool stencil = Texture::hasStencil(format);
	// TODO should handle these somehow... (need DEPTH_AND_STENCIL_SEPARATELY)
	// VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
	// VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL
	switch (type)
	{
	default:
		AKA_ASSERT(false, "Invalid attachment load op");
		[[fallthrough]];
	case ResourceAccessType::Undefined:
		return VK_IMAGE_LAYOUT_UNDEFINED;
	case ResourceAccessType::Resource:
		if(depth && stencil) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
#ifdef DEPTH_AND_STENCIL_SEPARATELY
		if (depth) return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
		else if (stencil) return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
#endif
		else return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ResourceAccessType::Attachment:
#ifdef DEPTH_AND_STENCIL_SEPARATELY
		if (depth && stencil) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		else if (depth) return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		else if (stencil) return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
#else
		if (depth || stencil) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
#endif
		else return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case ResourceAccessType::Storage:
		return VK_IMAGE_LAYOUT_GENERAL;
	case ResourceAccessType::CopySRC:
		return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	case ResourceAccessType::CopyDST:
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	case ResourceAccessType::Present:
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
}
VkAttachmentLoadOp VulkanContext::tovk(AttachmentLoadOp loadOp)
{
	switch (loadOp)
	{
	default:
		AKA_ASSERT(false, "Invalid attachment load op");
		[[fallthrough]];
	case AttachmentLoadOp::Clear:
		return VK_ATTACHMENT_LOAD_OP_CLEAR;
	case AttachmentLoadOp::Load:
		return VK_ATTACHMENT_LOAD_OP_LOAD;
	case AttachmentLoadOp::DontCare:
		return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
}
VkAttachmentStoreOp VulkanContext::tovk(AttachmentStoreOp loadOp)
{
	switch (loadOp)
	{
	default:
		AKA_ASSERT(false, "Invalid attachment store op");
		[[fallthrough]];
	case AttachmentStoreOp::Store:
		return VK_ATTACHMENT_STORE_OP_STORE;
	case AttachmentStoreOp::DontCare:
		return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
}

};
};

#endif