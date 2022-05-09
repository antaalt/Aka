#include "VulkanContext.h"

#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanFrameBuffer.h"
#include "VulkanProgram.h"
#include "VulkanPipeline.h"

#include "Platform/GLFW3/PlatformGLFW3.h"

#include <Aka/Core/Container/String.h>

#include <set>

#if defined(AKA_USE_VULKAN)

const char* vkGetErrorString(VkResult result)
{
	switch (result) {
	case VK_SUCCESS:
		return "VK_SUCCESS";
	case VK_NOT_READY:
		return "VK_NOT_READY";
	case VK_TIMEOUT:
		return "VK_TIMEOUT";
	case VK_EVENT_SET:
		return "VK_EVENT_SET";
	case VK_EVENT_RESET:
		return "VK_EVENT_RESET";
	case VK_INCOMPLETE:
		return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:
		return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL:
		return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR:
		return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "VK_ERROR_VALIDATION_FAILED_EXT";
	case VK_ERROR_INVALID_SHADER_NV:
		return "VK_ERROR_INVALID_SHADER_NV";
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
	case VK_ERROR_FRAGMENTATION_EXT:
		return "VK_ERROR_FRAGMENTATION_EXT";
	case VK_ERROR_NOT_PERMITTED_EXT:
		return "VK_ERROR_NOT_PERMITTED_EXT";
	case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
		return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
	case VK_RESULT_MAX_ENUM:
		return "VK_RESULT_MAX_ENUM";
	default:
		return "VK_UNKNOWN";
	}
}


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
		break;
	}
	return VK_FALSE;
}

#if !defined(_DEBUG)
static constexpr const bool enableValidationLayers = false;
#else
static constexpr const bool enableValidationLayers = true;
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

VkRenderPass VulkanContext::getRenderPass(const FramebufferState& fbDesc, VulkanRenderPassLayout layout)
{
	auto it = m_framebufferDesc.find(fbDesc);
	if (it != m_framebufferDesc.end())
		return it->second;
	if (layout != VulkanRenderPassLayout::Unknown)
	{
		VkRenderPass vk_renderPass = VulkanFramebuffer::createVkRenderPass(device, fbDesc, layout);
		m_framebufferDesc.insert(std::make_pair(fbDesc, vk_renderPass));
		return vk_renderPass;
	}
	else
	{
		VkRenderPass vk_renderPass = VulkanFramebuffer::createVkRenderPass(device, fbDesc, VulkanRenderPassLayout::Framebuffer);
		m_framebufferDesc.insert(std::make_pair(fbDesc, vk_renderPass));
		return vk_renderPass;
	}
}

VulkanContext::ShaderInputData VulkanContext::getDescriptorLayout(const ShaderBindingState& bindingsDesc)
{
	auto it = m_bindingDesc.find(bindingsDesc);
	if (it != m_bindingDesc.end())
		return it->second;
	ShaderInputData s;
	s.pool = VK_NULL_HANDLE;
	s.layout = VulkanProgram::createVkDescriptorSetLayout(device, bindingsDesc, &s.pool);

	m_bindingDesc.insert(std::make_pair(bindingsDesc, s));
	return s;
}

VkPipelineLayout VulkanContext::getPipelineLayout(const VkDescriptorSetLayout* layouts, uint32_t count)
{
	std::vector<VkDescriptorSetLayout> data(layouts, layouts + count);
	auto it = m_pipelineLayout.find(data);
	if (it != m_pipelineLayout.end())
		return it->second;

	// Create the layout of the pipeline following the provided descriptor set layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = count;
	pipelineLayoutCreateInfo.pSetLayouts = layouts;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	m_pipelineLayout.insert(std::make_pair(data, pipelineLayout));

	return pipelineLayout;
}

VulkanContext::VertexInputData VulkanContext::getVertexInputData(const VertexBindingState& verticesDesc)
{
	auto it = m_verticesDesc.find(verticesDesc);
	if (it != m_verticesDesc.end())
		return it->second;

	VertexInputData vertices{};
	vertices.bindings = VulkanGraphicPipeline::getVertexBindings(verticesDesc, vertices.attributes, VertexBindingState::MaxAttributes);

	m_verticesDesc.insert(std::make_pair(verticesDesc, vertices));
	return vertices;
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
	appInfo.apiVersion = VK_API_VERSION_1_0;

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

typedef bool (*PickPhysicalDeviceFunc)(const VkPhysicalDeviceProperties&, const VkPhysicalDeviceFeatures&);

VkPhysicalDevice VulkanContext::pickPhysicalDevice(PickPhysicalDeviceFunc isPhysicalDeviceSuitable)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		Logger::error("No physical device found.");
		return VK_NULL_HANDLE;
	}
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
	for (const VkPhysicalDevice& physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
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

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
	int32_t indexQueue = 0;
	int32_t graphicFamilyQueueIndex = -1;
	int32_t presentFamilyQueueIndex = -1;
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				graphicFamilyQueueIndex = indexQueue;
			}
			if (hasSurface)
			{
				VkBool32 presentSupport = false;
				VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, indexQueue, surface, &presentSupport));
				if (presentSupport)
				{
					presentFamilyQueueIndex = indexQueue;
					break;
				}
			}
		}
		indexQueue++;
	}
	if (graphicFamilyQueueIndex == -1 || (hasSurface && presentFamilyQueueIndex == -1))
	{
		Logger::error("No valid queue found.");
		return VK_NULL_HANDLE;
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
	// Queues
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies;
	uniqueQueueFamilies.insert(graphicFamilyQueueIndex);
	if (hasSurface)
		uniqueQueueFamilies.insert(presentFamilyQueueIndex);
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
	deviceFeatures.shaderFloat64 = VK_TRUE;
	deviceFeatures.multiDrawIndirect = VK_TRUE;

	VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
	indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
	indexingFeatures.pNext = nullptr;
	indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
	indexingFeatures.runtimeDescriptorArray = VK_TRUE;
	indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = &indexingFeatures;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
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
	VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));

	vkGetDeviceQueue(device, graphicFamilyQueueIndex, 0, &graphicQueue.queue);
	graphicQueue.index = graphicFamilyQueueIndex;
	if (hasSurface)
	{
		vkGetDeviceQueue(device, presentFamilyQueueIndex, 0, &presentQueue.queue);
		presentQueue.index = presentFamilyQueueIndex;
	}
	return device;
}

VkCommandPool createCommandPool(VkDevice device, uint32_t queueIndex)
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queueIndex;
	createInfo.flags = 0;

	VkCommandPool commandPool;
	vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);
	return commandPool;
}


void VulkanContext::initialize(PlatformDevice* platform, const GraphicConfig& config)
{
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
	// Custom extensions
	const char* requiredInstanceExtensions[] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
	};
	const size_t requiredInstanceExtensionCount = sizeof(requiredInstanceExtensions) / sizeof(*requiredInstanceExtensions);
	// All extensions
	const size_t instanceExtensionCount = requiredPlatformInstanceExtensionCount + requiredInstanceExtensionCount;
	const char** instanceExtensions = new const char*[instanceExtensionCount];
	for (size_t i = 0; i < requiredPlatformInstanceExtensionCount; i++)
		instanceExtensions[i] = requiredPlatformInstanceExtensions[i];
	for (size_t i = 0; i < requiredInstanceExtensionCount; i++)
		instanceExtensions[requiredPlatformInstanceExtensionCount + i] = requiredPlatformInstanceExtensions[i];
	// Device extensions
	const char* deviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	const size_t deviceExtensionCount = sizeof(deviceExtensions) / sizeof(*deviceExtensions);
	if (!hasValidationLayerSupport(validationLayers, validationLayerCount))
	{
		Logger::warn("Validation layers unsupported");
	}
	instance = createInstance(instanceExtensions, instanceExtensionCount);
	delete[] instanceExtensions;
	surface = createSurface(platform);

	physicalDevice = pickPhysicalDevice([](const VkPhysicalDeviceProperties& properties, const VkPhysicalDeviceFeatures& features) {
		return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && features.geometryShader && features.samplerAnisotropy;
	});
	device = createLogicalDevice(deviceExtensions, deviceExtensionCount);

	commandPool = createCommandPool(device, graphicQueue.index);
}

void VulkanContext::shutdown()
{
	for (auto& rp : m_verticesDesc)
	{
		// nothing to clear.
	}
	for (auto& rp : m_bindingDesc)
	{
		vkDestroyDescriptorPool(device, rp.second.pool, nullptr);
		vkDestroyDescriptorSetLayout(device, rp.second.layout, nullptr);
		//vkDestroyPipelineLayout(device, rp.second.pipelineLayout, nullptr);
	}
	for (auto& rp : m_pipelineLayout)
	{
		vkDestroyPipelineLayout(device, rp.second, nullptr);
	}
	for (auto& rp : m_framebufferDesc)
	{
		vkDestroyRenderPass(device, rp.second, nullptr);
	}
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(device, nullptr);
	// physical device
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
	throw std::runtime_error("failed to find suitable memory type!");
}

VkFormat VulkanContext::tovk(TextureFormat format)
{
	switch (format)
	{
	default:
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
	case BufferType::Uniform:
		return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	case BufferType::ShaderStorage:
		return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}
}

};
};

#endif