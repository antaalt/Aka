#include "VulkanInstance.h"

#include <Aka/Memory/Allocator.h>

#include "VulkanGraphicDevice.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <renderdoc_app.h>

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

static bool hasValidationLayerSupport(const char* const* validationLayers, size_t count)
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
	// VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT requires extension VK_EXT_device_address_binding_report
	// VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT is too verbose
	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
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

VkInstance VulkanInstance::createInstance(const char** instanceExtensions, size_t instanceExtensionCount)
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

bool VulkanInstance::isValidationLayerEnabled()
{
	return s_enableValidationLayers;
}
uint32_t VulkanInstance::getValidationLayerCount()
{
	return s_validationLayerCount;
}
const char *const* VulkanInstance::getValidationLayers()
{
	return s_validationLayers;
}
uint32_t VulkanInstance::getRequiredDeviceExtensionCount()
{
	return s_requiredDeviceExtensionCount;
}
const char* const* VulkanInstance::getRequiredDeviceExtensions()
{
	return s_requiredDeviceExtensions;
}

uint32_t VulkanInstance::getPhysicalDeviceCount() const
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
	return deviceCount;
}

VulkanInstance::VulkanInstance() :
	m_instance(VK_NULL_HANDLE),
	m_debugMessenger(VK_NULL_HANDLE)
{
}
VulkanInstance::~VulkanInstance()
{
}
void VulkanInstance::initialize()
{
#ifdef ENABLE_RENDERDOC_CAPTURE
	// TODO: reenable entry point.
	if (s_enableValidationLayers)// asBool(m_physicalDeviceFeatures & PhysicalDeviceFeatures::RenderDocAttachment))
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
	// We are using Vulkan 1.2 in this backend.
	//clipSpacePositive = true; // VK clip space is [0, 1]
	//originTextureBottomLeft = false; // VK start reading texture at top left.
	//originUVBottomLeft = false; // VK UV origin is top left
	//renderAxisYUp = false; // VK render axis y is down
	// 
	// Create instance & device
	// Platform instance extension
	uint32_t requiredPlatformInstanceExtensionCount = 0;
	const char** requiredPlatformInstanceExtensions = glfwGetRequiredInstanceExtensions(&requiredPlatformInstanceExtensionCount); // pointer owned by glfw
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
	m_instance = createInstance(instanceExtensions.data(), instanceExtensionCount);
	m_debugMessenger = createDebugMessenger(m_instance);
}
void VulkanInstance::shutdown()
{
	m_surfacePool.release([this](const VulkanSurface& res) { Logger::warn("Leaking surface ", res.name); this->destroy(SurfaceHandle{ &res }); });
	// physical device
	vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, getVkAllocator());
	vkDestroyInstance(m_instance, getVkAllocator());
}
SurfaceHandle VulkanInstance::createSurface(const char* name, PlatformWindow* window)
{
	VulkanSurface* vk_surface = m_surfacePool.acquire(name, window);
	vk_surface->create(this);
	return SurfaceHandle{ vk_surface };
}
const Surface* VulkanInstance::get(SurfaceHandle handle)
{
	return static_cast<const Surface*>(handle.__data);
}
void VulkanInstance::destroy(SurfaceHandle handle)
{
	if (handle == SurfaceHandle::null) return;

	VulkanSurface* vk_surface = getVk<VulkanSurface>(handle);
	vk_surface->destroy(this);

	m_surfacePool.release(vk_surface);
}

// Should have generic limits + features data that are computed from vk & D3D with common selection process based on generic data.
std::tuple<uint32_t, PhysicalDeviceFeatures, PhysicalDeviceLimits> getPhysicalDeviceScore(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceProperties2& properties)
{
	VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomicFloatFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT };
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

GraphicDevice* VulkanInstance::pick(PhysicalDeviceFeatures _requestedFeatures, PlatformWindow* window)
{
	uint32_t deviceCount = 0;
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));
	if (deviceCount == 0)
	{
		return nullptr; // No device available.
	}
	VulkanSurface* surface = getVk<VulkanSurface>(window->surface());
	VkSurfaceKHR vk_surface = surface->vk_surface;
	Vector<VkPhysicalDevice> physicalDevices(deviceCount);
	uint32_t maxScore = 0;
	VkPhysicalDevice physicalDevicePicked = VK_NULL_HANDLE;
	PhysicalDeviceFeatures physicalDeviceFeatures = PhysicalDeviceFeatures::None;
	PhysicalDeviceLimits physicalDeviceLimits = {};
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data()));
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
		bool hasValidSwapchain = isSwapchainAdequate(physicalDevice, vk_surface);
		bool hasValidQueues = areQueuesAdequate(physicalDevice, vk_surface);
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
		return nullptr;
	}
	else
	{
		return mem::akaNew<VulkanGraphicDevice>(AllocatorMemoryType::Object, AllocatorCategory::Graphic, this, surface,physicalDevicePicked, physicalDeviceFeatures, physicalDeviceLimits);
	}
}
void VulkanInstance::destroy(GraphicDevice* device)
{
	mem::akaDelete(device);
}

}
}

#endif