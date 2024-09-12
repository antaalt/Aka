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
	case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
		message += "[device-address-binding]";
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
		Logger::debug(message, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		Logger::warn(message, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		AKA_ASSERT(false, pCallbackData->pMessage);
		Logger::error(message, pCallbackData->pMessage);
		break;
	}
	return VK_FALSE;
}

#if defined(AKA_DEBUG)
static constexpr const bool s_enableValidationLayers = true;
#else
static constexpr const bool s_enableValidationLayers = false;
#endif
static constexpr const char* s_validationLayers[] = {
	"VK_LAYER_KHRONOS_validation",
	"VK_LAYER_KHRONOS_synchronization2"
};
static constexpr const size_t s_validationLayerCount = sizeof(s_validationLayers) / sizeof(*s_validationLayers);


static constexpr const char* s_requiredInstanceExtensions[] = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME, // Replace old VK_EXT_debug_marker & VK_EXT_debug_report 
	VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
};
static constexpr const size_t s_requiredInstanceExtensionCount = countof(s_requiredInstanceExtensions);
static constexpr const char* s_requiredDeviceExtensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
};
static constexpr const size_t s_requiredDeviceExtensionCount = countof(s_requiredDeviceExtensions);

static bool isDeviceSupportingRequiredExtensions(VkPhysicalDevice physicalDevice)
{
	uint32_t extensionCount = 0;
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr));
	Vector<VkExtensionProperties> availableDeviceExtensions(extensionCount);
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableDeviceExtensions.data()));
	for (size_t i = 0; i < s_requiredDeviceExtensionCount; i++)
	{
		bool foundExtension = false;
		for (const VkExtensionProperties& extension : availableDeviceExtensions)
		{
			if (String::compare(extension.extensionName, s_requiredDeviceExtensions[i]) == 0)
			{
				foundExtension = true;
				break;
			}
		}
		if (!foundExtension)
		{
			Logger::error("Device extension '", s_requiredDeviceExtensions[i], "' not available.");
			return false;
		}
	}
	return true;
}

static bool hasValidationLayerSupport(const char*const* validationLayers, size_t count)
{
	uint32_t availableLayerCount;
	VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr));

	Vector<VkLayerProperties> availableLayers(availableLayerCount);
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
	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;//| VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
	debugCreateInfo.pfnUserCallback = debugCallback;
	debugCreateInfo.pUserData = nullptr; // Optional
}

VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance)
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	populateCreateInfo(createInfo);
	VkDebugUtilsMessengerEXT messenger;
	VK_CHECK_RESULT(vkCreateDebugUtilsMessengerEXT(instance, &createInfo, getVkAllocator(), &messenger));
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
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, getVkAllocator(), &pipelineLayout));

	m_pipelineLayout.insert(std::make_pair(pair, pipelineLayout));

	return pipelineLayout;
}

VkInstance VulkanContext::createInstance(const char** instanceExtensions, size_t instanceExtensionCount)
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	Vector<VkExtensionProperties> extensions(extensionCount);
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
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensionCount);
	createInfo.ppEnabledExtensionNames = instanceExtensions;
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (s_enableValidationLayers)
	{

		createInfo.enabledLayerCount = static_cast<uint32_t>(s_validationLayerCount);
		createInfo.ppEnabledLayerNames = s_validationLayers;
		populateCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}
	VkInstance instance;
	VK_CHECK_RESULT(vkCreateInstance(&createInfo, getVkAllocator(), &instance));
	return instance;
}

VkSurfaceKHR VulkanContext::createSurface(PlatformDevice* platform)
{
	VkSurfaceKHR surface;
	VK_CHECK_RESULT(glfwCreateWindowSurface(
		instance,
		reinterpret_cast<PlatformGLFW3*>(platform)->getGLFW3Handle(),
		getVkAllocator(),
		&surface
	));
	return surface;
}

const char** VulkanContext::getPlatformRequiredInstanceExtension(const PlatformDevice* platform, uint32_t* count)
{
	// Pointer is owned by glfw here.
	return glfwGetRequiredInstanceExtensions(count);
}

bool isSwapchainAdequate(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	VkSurfaceCapabilitiesKHR capabilities{};
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities));

	uint32_t formatCount;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr));
	Vector<VkSurfaceFormatKHR> formats(formatCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data()));
	if (formatCount == 0)
	{
		Logger::error("No surface format found for swapchain.");
		return false;
	}
	
	uint32_t presentModeCount;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
	Vector<VkPresentModeKHR> presentModes(presentModeCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()));
	if (presentModeCount == 0)
	{
		Logger::error("No present mode found for swapchain.");
		return false;
	}
	return formatCount > 0 && presentModeCount > 0;
}

bool areQueuesAdequate(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	Vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	const bool hasSurface = surface != VK_NULL_HANDLE;
	bool hasAsyncCompute = false;
	bool hasAsyncCopy = false;
	bool hasGraphicsAndPresent = false;
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
				VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, iQueue, surface, &presentSupport));
				if (presentSupport == VK_TRUE)
				{
					queueFamilySlotCount[iQueue]++;
					hasGraphicsAndPresent = true;
					break;
				}
			}
			else
			{
				hasGraphicsAndPresent = true;
				break;
			}
		}
	}
	if (!hasGraphicsAndPresent)
	{
		Logger::error("Could not find valid graphics & present queue.");
		return false;
	}
	for (uint32_t iQueue = 0; iQueue < queueFamilyCount; ++iQueue)
	{
		const VkQueueFamilyProperties& queueFamily = queueFamilies[iQueue];
		const bool isGraphicQueue = (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT);
		const bool isComputeQueue = (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT);
		const bool isCopyQueue = (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT);
		if (!hasAsyncCompute && isComputeQueue && !isGraphicQueue && (queueFamilySlotCount[iQueue] < queueFamily.queueCount))
		{
			hasAsyncCompute = true;
		}
		if (!hasAsyncCopy && isCopyQueue && !isGraphicQueue && (queueFamilySlotCount[iQueue] < queueFamily.queueCount))
		{
			hasAsyncCopy = true;
		}
	}
	if (!hasAsyncCompute)
	{
		Logger::error("Could not find valid async compute queue.");
		return false;
	}
	if (!hasAsyncCopy)
	{
		Logger::error("Could not find valid async copy queue.");
		return false;
	}
	return hasGraphicsAndPresent && hasAsyncCompute && hasAsyncCopy;
}
// Should have generic limits + features data that are computed from vk & D3D with common selection process based on generic data.
std::tuple<uint32_t, PhysicalDeviceFeatures, PhysicalDeviceLimits> getPhysicalDeviceScore(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceProperties2& properties)
{
	VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomicFloatFeatures { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT };
	VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR fragmentShaderBarycentricsFeature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR };
	VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };
	VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES };
	VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES };
	fragmentShaderBarycentricsFeature.pNext = &atomicFloatFeatures;
	meshShaderFeatures.pNext = &fragmentShaderBarycentricsFeature;
	timelineSemaphoreFeatures.pNext = &meshShaderFeatures;
	indexingFeatures.pNext = &timelineSemaphoreFeatures;

	VkPhysicalDeviceFeatures2 features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	features.pNext = &indexingFeatures;
	vkGetPhysicalDeviceFeatures2(physicalDevice, &features);
	PhysicalDeviceFeatures supportedFeatureMask = PhysicalDeviceFeatures::All;
	uint32_t score = 0;
	switch (properties.properties.deviceType)
	{
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		score += 100;
		break;
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		score += 10;
		break;
	default:
		Logger::error(properties.properties.deviceName, ": Device type invalid. Skipping.");
		return std::make_tuple(0, PhysicalDeviceFeatures::None, PhysicalDeviceLimits{});
	};
	//score += properties.limits.maxImageDimension2D;

	bool requiredFeatures = true;
	requiredFeatures &= features.features.samplerAnisotropy == VK_TRUE;
	requiredFeatures &= features.features.fillModeNonSolid == VK_TRUE; // VK_POLYGON_MODE_LINE
	requiredFeatures &= features.features.fragmentStoresAndAtomics == VK_TRUE;
	requiredFeatures &= features.features.shaderFloat64 == VK_TRUE;
	requiredFeatures &= features.features.multiDrawIndirect == VK_TRUE;

	// Renderdoc seems to require these features:
	bool renderDocRequiredFeatures = true;
	renderDocRequiredFeatures &= features.features.geometryShader == VK_TRUE; // render doc
	renderDocRequiredFeatures &= features.features.sampleRateShading == VK_TRUE; // render doc
	if (!renderDocRequiredFeatures)
	{
		Logger::warn(properties.properties.deviceName, ": Does not support required extension for renderdoc.");
		supportedFeatureMask = ~PhysicalDeviceFeatures::RenderDocAttachment;
	}

	VkBaseOutStructure* base = reinterpret_cast<VkBaseOutStructure*>(features.pNext);
	while (base != nullptr)
	{
		switch (base->sType)
		{
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT: {
			bool meshShaderSupport = true;
			VkPhysicalDeviceMeshShaderFeaturesEXT* meshShaderFeatures = reinterpret_cast<VkPhysicalDeviceMeshShaderFeaturesEXT*>(base);
			meshShaderSupport &= meshShaderFeatures->taskShader == VK_TRUE;
			meshShaderSupport &= meshShaderFeatures->meshShader == VK_TRUE;
			if (!meshShaderSupport)
			{
				Logger::warn(properties.properties.deviceName, ": Extension mesh shader not supported.");
				supportedFeatureMask &= ~PhysicalDeviceFeatures::MeshShader;
			}
			break;
		}
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES: {
			// This is here to emulate DX12 semaphore, so use it as required extension.
			VkPhysicalDeviceTimelineSemaphoreFeatures* timelineSemaphoreFeatures = reinterpret_cast<VkPhysicalDeviceTimelineSemaphoreFeatures*>(base);
			requiredFeatures &= timelineSemaphoreFeatures->timelineSemaphore == VK_TRUE;
			break;
		}
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES: {
			bool bindlessSupport = true;
			VkPhysicalDeviceDescriptorIndexingFeatures* indexingFeatures = reinterpret_cast<VkPhysicalDeviceDescriptorIndexingFeatures*>(base);
			bindlessSupport &= indexingFeatures->descriptorBindingPartiallyBound == VK_TRUE;
			bindlessSupport &= indexingFeatures->runtimeDescriptorArray == VK_TRUE;
			bindlessSupport &= indexingFeatures->shaderSampledImageArrayNonUniformIndexing == VK_TRUE;
			bindlessSupport &= indexingFeatures->descriptorBindingSampledImageUpdateAfterBind == VK_TRUE;
			bindlessSupport &= indexingFeatures->descriptorBindingVariableDescriptorCount == VK_TRUE;
			if (!bindlessSupport)
			{
				Logger::warn(properties.properties.deviceName, ": Extension descriptor indexing not supported.");
				supportedFeatureMask &= ~PhysicalDeviceFeatures::BindlessResources;
			}
			break;
		}
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR: {
			VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR* barycentricFeatures = reinterpret_cast<VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR*>(base);
			bool barycentricSupport = barycentricFeatures->fragmentShaderBarycentric == VK_TRUE;
			if (!barycentricSupport)
			{
				Logger::warn(properties.properties.deviceName, ": Extension barycentric not supported.");
				supportedFeatureMask &= ~PhysicalDeviceFeatures::Barycentric;
			}
			break;
		}
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT: {
			VkPhysicalDeviceShaderAtomicFloatFeaturesEXT* atomicFloatFeatures = reinterpret_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT*>(base);
			bool atomicFloatSupport = true;
			// TODO: check only buffers ?
			atomicFloatSupport &= atomicFloatFeatures->shaderBufferFloat32Atomics == VK_TRUE;
			atomicFloatSupport &= atomicFloatFeatures->shaderBufferFloat32AtomicAdd == VK_TRUE;
			if (!atomicFloatSupport)
			{
				Logger::warn(properties.properties.deviceName, ": Extension atomic float not supported.");
				supportedFeatureMask &= ~PhysicalDeviceFeatures::AtomicFloat;
			}
			break;
		}
		default:
			break;
		}
		base = base->pNext;
	}
	PhysicalDeviceLimits limits{};
	limits.maxTexture1DSize = properties.properties.limits.maxImageDimension1D;
	limits.maxTexture2DSize = properties.properties.limits.maxImageDimension2D;
	limits.maxTexture3DSize = properties.properties.limits.maxImageDimension3D;
	limits.maxTextureCubeSize = properties.properties.limits.maxImageDimensionCube;
	limits.maxTextureLayers = properties.properties.limits.maxImageArrayLayers;
	limits.maxPushConstantSize = properties.properties.limits.maxPushConstantsSize;
	for (uint32_t i = 0; i < 3; i++)
		limits.maxComputeWorkgroupCount[i] = properties.properties.limits.maxComputeWorkGroupCount[i];
	limits.maxComputeWorkGroupInvocations = properties.properties.limits.maxComputeWorkGroupInvocations;
	for (uint32_t i = 0; i < 3; i++)
		limits.maxComputeWorkgroupSize[i] = properties.properties.limits.maxComputeWorkGroupSize[i];

	VkBaseOutStructure* baseProp = reinterpret_cast<VkBaseOutStructure*>(properties.pNext);
	while (baseProp != nullptr)
	{
		switch (baseProp->sType)
		{
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT: {
			VkPhysicalDeviceMeshShaderPropertiesEXT* meshShaderProperties = reinterpret_cast<VkPhysicalDeviceMeshShaderPropertiesEXT*>(baseProp);

			for (uint32_t i = 0; i < 3; i++)
				limits.maxMeshShaderWorkgroupCount[i] = meshShaderProperties->maxMeshWorkGroupCount[i];
			limits.maxMeshShaderWorkGroupInvocations = meshShaderProperties->maxMeshWorkGroupInvocations;
			for (uint32_t i = 0; i < 3; i++)
				limits.maxMeshShaderWorkgroupSize[i] = meshShaderProperties->maxMeshWorkGroupSize[i];
			break;
		}
		default: 
			break;
		}
		baseProp = baseProp->pNext;
	}
	
	if (!requiredFeatures)
	{
		Logger::error(properties.properties.deviceName, ": Some required features are not supported. Skipping device.");
		return std::make_tuple(0, supportedFeatureMask, limits);
	}
	return std::make_tuple(score, supportedFeatureMask, limits);
}

std::tuple<VkPhysicalDevice, PhysicalDeviceFeatures, PhysicalDeviceLimits> VulkanContext::pickPhysicalDevice(PhysicalDeviceFeatures _requestedFeatures)
{
	uint32_t deviceCount = 0;
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
	if (deviceCount == 0)
	{
		AKA_CRASH("No physical device found");
		return std::make_tuple(VK_NULL_HANDLE, PhysicalDeviceFeatures::None, PhysicalDeviceLimits{});
	}
	Vector<VkPhysicalDevice> physicalDevices(deviceCount);
	uint32_t maxScore = 0;
	VkPhysicalDevice physicalDevicePicked = VK_NULL_HANDLE;
	PhysicalDeviceFeatures physicalDeviceFeatures = PhysicalDeviceFeatures::None;
	PhysicalDeviceLimits physicalDeviceLimits = {};
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()));
	for (const VkPhysicalDevice& physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT };
		VkPhysicalDeviceProperties2 deviceProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
		deviceProperties.pNext = &meshShaderProperties;
		vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);
		// A score of zero means invalid device (unsupported mandatory feature).
		uint32_t score = 0;
		PhysicalDeviceFeatures supportedFeatureMask = PhysicalDeviceFeatures::None;
		PhysicalDeviceLimits supportedDeviceLimits = PhysicalDeviceLimits{};
		std::tie(score, supportedFeatureMask, supportedDeviceLimits) = getPhysicalDeviceScore(physicalDevice, deviceProperties);
		bool hasValidScore = score > 0;
		bool hasRequestedFeatures = (_requestedFeatures & supportedFeatureMask) == _requestedFeatures;
		bool hasValidSwapchain = isSwapchainAdequate(physicalDevice, surface);
		bool hasValidQueues = areQueuesAdequate(physicalDevice, surface);
		bool hasValidExtensions = isDeviceSupportingRequiredExtensions(physicalDevice);
		if (hasValidScore && hasRequestedFeatures && hasValidSwapchain && hasValidQueues && hasValidExtensions)
		{
			Logger::info("Candidate physical device detected: ", deviceProperties.properties.deviceName, " (Score: ", score, ")");
			if (score > maxScore)
			{
				maxScore = score;
				physicalDevicePicked = physicalDevice;
				physicalDeviceFeatures = _requestedFeatures;
				physicalDeviceLimits = supportedDeviceLimits;
			}
		}
		else
		{
			Vector<String> reasons;
			if (!hasValidScore) reasons.append("Invalid GPU");
			if (!hasRequestedFeatures)
			{
				PhysicalDeviceFeatures unsupportedFeatures = _requestedFeatures & ~supportedFeatureMask;
				for (PhysicalDeviceFeatures features : EnumBitRange(unsupportedFeatures))
				{
					reasons.append(String::format("Required physical device feature not supported: %s", toString(features)));
				}
			}
			if (!hasValidSwapchain) reasons.append("Invalid swapchain");
			if (!hasValidQueues) reasons.append("Invalid queues");
			if (!hasValidExtensions) reasons.append("Invalid required extensions");
			String formattedReason;
			for (size_t i = 0; i < reasons.size(); i++)
			{
				formattedReason.append(reasons[i]);
				if (i != reasons.size() - 1)
					formattedReason.append(", ");
			}
			Logger::warn("Candidate physical device ", deviceProperties.properties.deviceName, " rejected for following reason(s): ", formattedReason);
		}
	}
	if (physicalDevicePicked == VK_NULL_HANDLE)
	{
		AKA_CRASH("No compatible physical device found");
		return std::make_tuple(VK_NULL_HANDLE, PhysicalDeviceFeatures::None, PhysicalDeviceLimits{});
	}
	else
	{
		return std::make_tuple(physicalDevicePicked, physicalDeviceFeatures, physicalDeviceLimits);
	}
}

VkDevice VulkanContext::createLogicalDevice(const char* const* deviceExtensions, size_t deviceExtensionCount)
{
	const bool hasSurface = surface != VK_NULL_HANDLE;
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
				VkBool32 presentSupport = VK_FALSE;
				VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, iQueue, surface, &presentSupport));
				if (presentSupport == VK_TRUE)
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
		const bool isGraphicQueue = (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT);
		const bool isComputeQueue = (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT);
		const bool isCopyQueue = (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT);
		if (!isQueuePicked[EnumToIndex(QueueType::Compute)] && isComputeQueue && !isGraphicQueue && (queueFamilySlotCount[iQueue] < queueFamily.queueCount))
		{
			isQueuePicked[EnumToIndex(QueueType::Compute)] = true;
			queues[EnumToIndex(QueueType::Compute)].familyIndex = iQueue;
			queues[EnumToIndex(QueueType::Compute)].index = queueFamilySlotCount[iQueue]++;
			AKA_ASSERT(queueFamilySlotCount[iQueue] <= queueFamily.queueCount, "Too many queues");
		}
		if (!isQueuePicked[EnumToIndex(QueueType::Copy)] && isCopyQueue && !isGraphicQueue && (queueFamilySlotCount[iQueue] < queueFamily.queueCount))
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
	HashSet<uint32_t> uniqueQueueFamilies;
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
		uniqueQueueFamilies.insert(queues[i].familyIndex);
	if (hasSurface)
		uniqueQueueFamilies.insert(presentQueue.familyIndex);

	Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	HashMap<uint32_t, Vector<float>> priorities;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		// Only handle separated queue here, ignore present shared with graphics.
		for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
			if (queues[i].familyIndex == queueFamily)
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
	if (asBool(physicalDeviceFeatures & PhysicalDeviceFeatures::BindlessResources))
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
	if (asBool(physicalDeviceFeatures & PhysicalDeviceFeatures::MeshShader))
	{
		meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
		meshShaderFeatures.pNext = &timelineSemaphoreFeatures;
		meshShaderFeatures.taskShader = VK_TRUE;
		meshShaderFeatures.meshShader = VK_TRUE;
		next = &meshShaderFeatures;
	}

	// VK_VERSION_1_0
	VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR fragmentShaderBarycentricsFeature{};
	if (asBool(physicalDeviceFeatures & PhysicalDeviceFeatures::Barycentric))
	{
		fragmentShaderBarycentricsFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR;
		fragmentShaderBarycentricsFeature.pNext = next;
		fragmentShaderBarycentricsFeature.fragmentShaderBarycentric = VK_TRUE;
		next = &fragmentShaderBarycentricsFeature;
	}

	// VK_VERSION_1_1
	VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomicFloatFeatures{};
	if (asBool(physicalDeviceFeatures & PhysicalDeviceFeatures::AtomicFloat))
	{
		atomicFloatFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT;
		atomicFloatFeatures.pNext = next;
		atomicFloatFeatures.shaderBufferFloat32AtomicAdd = VK_TRUE;
		atomicFloatFeatures.shaderBufferFloat32Atomics = VK_TRUE;
		next = &atomicFloatFeatures;
	}

	// VK_VERSION_1_1
	VkPhysicalDeviceFeatures2 deviceFeatures {};
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

	VkDeviceCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = &deviceFeatures;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = NULL; // VK_VERSION_1_0
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionCount);
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	if (s_enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(s_validationLayerCount);
		createInfo.ppEnabledLayerNames = s_validationLayers;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}
	VkDevice device;
	VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &createInfo, getVkAllocator(), &device));

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
	vkCreateCommandPool(device, &createInfo, getVkAllocator(), &commandPool);
	return commandPool;
}


bool VulkanContext::initialize(PlatformDevice* platform, const GraphicConfig& config)
{
	// We are using Vulkan 1.2 in this backend.
	//clipSpacePositive = true; // VK clip space is [0, 1]
	//originTextureBottomLeft = false; // VK start reading texture at top left.
	//originUVBottomLeft = false; // VK UV origin is top left
	//renderAxisYUp = false; // VK render axis y is down
	m_platform = platform;
	// Create instance & device
	// Platform instance extension
	uint32_t requiredPlatformInstanceExtensionCount = 0;
	const char** requiredPlatformInstanceExtensions = getPlatformRequiredInstanceExtension(platform, &requiredPlatformInstanceExtensionCount);
	AKA_ASSERT(requiredPlatformInstanceExtensions != nullptr, "GLFW failed to initialize instance extensions.");
	// Custom instance extensions
	const size_t requiredInstanceExtensionCount = sizeof(s_requiredInstanceExtensions) / sizeof(*s_requiredInstanceExtensions);
	// All instance extensions
	const size_t instanceExtensionCount = requiredPlatformInstanceExtensionCount + requiredInstanceExtensionCount;
	Vector<const char*> instanceExtensions(instanceExtensionCount);
	for (size_t i = 0; i < requiredPlatformInstanceExtensionCount; i++)
		instanceExtensions[i] = requiredPlatformInstanceExtensions[i];
	for (size_t i = 0; i < requiredInstanceExtensionCount; i++)
		instanceExtensions[requiredPlatformInstanceExtensionCount + i] = s_requiredInstanceExtensions[i];

	// Check platform & required instance extensions
	uint32_t extensionCount = 0;
	VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
	Vector<VkExtensionProperties> availableInstanceExtensions(extensionCount);
	VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableInstanceExtensions.data()));
	for (size_t i = 0; i < instanceExtensionCount; i++)
	{
		bool foundExtension = false;
		for (const VkExtensionProperties& extension : availableInstanceExtensions)
		{
			if (String::compare(extension.extensionName, instanceExtensions[i]) == 0)
			{
				foundExtension = true;
				break;
			}
		}
		if (!foundExtension)
		{
			Logger::error("Instance extension '", instanceExtensions[i], "' not available.");
		}
	}
	// Validation layers
	if (!hasValidationLayerSupport(s_validationLayers, s_validationLayerCount))
	{
		Logger::warn("Validation layers unsupported");
	}
	instance = createInstance(instanceExtensions.data(), instanceExtensionCount);
	debugMessenger = createDebugMessenger(instance);
	surface = createSurface(platform);

	std::tie(physicalDevice, physicalDeviceFeatures, physicalDeviceLimits) = pickPhysicalDevice(config.features);

	if (physicalDevice != VK_NULL_HANDLE)
	{
		// List all used extensions & device features extension that are requested.
		Vector<const char*> usedDeviceExtensions;
		for (uint32_t i = 0; i < s_requiredDeviceExtensionCount; i++)
		{
			usedDeviceExtensions.append(s_requiredDeviceExtensions[i]);
		}
		if (asBool(physicalDeviceFeatures & PhysicalDeviceFeatures::MeshShader))
		{
			usedDeviceExtensions.append(VK_EXT_MESH_SHADER_EXTENSION_NAME);
		}
		if (asBool(physicalDeviceFeatures & PhysicalDeviceFeatures::AtomicFloat))
		{
			usedDeviceExtensions.append(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
		}
		if (asBool(physicalDeviceFeatures & PhysicalDeviceFeatures::BindlessResources))
		{
			usedDeviceExtensions.append(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
		}
		if (asBool(physicalDeviceFeatures & PhysicalDeviceFeatures::Barycentric))
		{
			usedDeviceExtensions.append(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME);
		}
		device = createLogicalDevice(usedDeviceExtensions.data(), (uint32_t)usedDeviceExtensions.size());

		for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
		{
			commandPool[i] = createCommandPool(device, queues[i].familyIndex);
			setDebugName(device, commandPool[i], "MainCommandPool", i);
		}
		return true;
	}
	else
	{
		return false;
	}
}

void VulkanContext::shutdown()
{
	for (auto& rp : m_descriptorSetLayouts)
	{
		vkDestroyDescriptorSetLayout(device, rp.second, getVkAllocator());
		//vkDestroyPipelineLayout(device, rp.second.pipelineLayout, nullptr);
	}
	for (auto& rp : m_pipelineLayout)
	{
		vkDestroyPipelineLayout(device, rp.second, getVkAllocator());
	}
	for (auto& rp : m_renderPassState)
	{
		vkDestroyRenderPass(device, rp.second, getVkAllocator());
	}
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
		vkDestroyCommandPool(device, commandPool[i], getVkAllocator());
	vkDestroySurfaceKHR(instance, surface, getVkAllocator());
	vkDestroyDevice(device, getVkAllocator());
	// physical device
	vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, getVkAllocator());
	vkDestroyInstance(instance, getVkAllocator());
}

uint32_t VulkanContext::getPhysicalDeviceCount() const
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	return deviceCount;
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
	AKA_ASSERT(false, "Failed to find suitable memory type!");
	return 0;
}

VkFormat VulkanContext::tovk(TextureFormat format)
{
	switch (format)
	{
	default:
		AKA_ASSERT(false, "Invalid texture format");
		return VK_FORMAT_UNDEFINED;
	case TextureFormat::Swapchain:
		return VK_FORMAT_B8G8R8A8_UNORM; // TODO: retrieve dynamically
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

VkBufferUsageFlags VulkanContext::tovk(BufferType type)
{
	VkBufferUsageFlags flags = 0;
	if (has(type, BufferType::Vertex))
		flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	if (has(type, BufferType::Index))
		flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if (has(type, BufferType::Uniform))
		flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	if (has(type, BufferType::Storage))
		flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	if (has(type, BufferType::Indirect))
		flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	return flags;
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