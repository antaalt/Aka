#include "VulkanDebug.h"

#if defined(AKA_USE_VULKAN)
namespace aka {
namespace gfx {

const char* vkGetObjectName(VkObjectType objectType)
{
	switch (objectType)
	{
	case VK_OBJECT_TYPE_UNKNOWN:
		return "VK_OBJECT_TYPE_UNKNOWN";
	case VK_OBJECT_TYPE_INSTANCE:
		return "VK_OBJECT_TYPE_INSTANCE";
	case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
		return "VK_OBJECT_TYPE_PHYSICAL_DEVICE";
	case VK_OBJECT_TYPE_DEVICE:
		return "VK_OBJECT_TYPE_DEVICE";
	case VK_OBJECT_TYPE_QUEUE:
		return "VK_OBJECT_TYPE_QUEUE";
	case VK_OBJECT_TYPE_SEMAPHORE:
		return "VK_OBJECT_TYPE_SEMAPHORE";
	case VK_OBJECT_TYPE_COMMAND_BUFFER:
		return "VK_OBJECT_TYPE_COMMAND_BUFFER";
	case VK_OBJECT_TYPE_FENCE:
		return "VK_OBJECT_TYPE_FENCE";
	case VK_OBJECT_TYPE_DEVICE_MEMORY:
		return "VK_OBJECT_TYPE_DEVICE_MEMORY";
	case VK_OBJECT_TYPE_BUFFER:
		return "VK_OBJECT_TYPE_BUFFER";
	case VK_OBJECT_TYPE_IMAGE:
		return "VK_OBJECT_TYPE_IMAGE";
	case VK_OBJECT_TYPE_EVENT:
		return "VK_OBJECT_TYPE_EVENT";
	case VK_OBJECT_TYPE_QUERY_POOL:
		return "VK_OBJECT_TYPE_QUERY_POOL";
	case VK_OBJECT_TYPE_BUFFER_VIEW:
		return "VK_OBJECT_TYPE_BUFFER_VIEW";
	case VK_OBJECT_TYPE_IMAGE_VIEW:
		return "VK_OBJECT_TYPE_IMAGE_VIEW";
	case VK_OBJECT_TYPE_SHADER_MODULE:
		return "VK_OBJECT_TYPE_SHADER_MODULE";
	case VK_OBJECT_TYPE_PIPELINE_CACHE:
		return "VK_OBJECT_TYPE_PIPELINE_CACHE";
	case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
		return "VK_OBJECT_TYPE_PIPELINE_LAYOUT";
	case VK_OBJECT_TYPE_RENDER_PASS:
		return "VK_OBJECT_TYPE_RENDER_PASS";
	case VK_OBJECT_TYPE_PIPELINE:
		return "VK_OBJECT_TYPE_PIPELINE";
	case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
		return "VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT";
	case VK_OBJECT_TYPE_SAMPLER:
		return "VK_OBJECT_TYPE_SAMPLER";
	case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
		return "VK_OBJECT_TYPE_DESCRIPTOR_POOL";
	case VK_OBJECT_TYPE_DESCRIPTOR_SET:
		return "VK_OBJECT_TYPE_DESCRIPTOR_SET";
	case VK_OBJECT_TYPE_FRAMEBUFFER:
		return "VK_OBJECT_TYPE_FRAMEBUFFER";
	case VK_OBJECT_TYPE_COMMAND_POOL:
		return "VK_OBJECT_TYPE_COMMAND_POOL";
	case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:
		return "VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION";
	case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
		return "VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE";
	case VK_OBJECT_TYPE_SURFACE_KHR:
		return "VK_OBJECT_TYPE_SURFACE_KHR";
	case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
		return "VK_OBJECT_TYPE_SWAPCHAIN_KHR";
	case VK_OBJECT_TYPE_DISPLAY_KHR:
		return "VK_OBJECT_TYPE_DISPLAY_KHR";
	case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
		return "VK_OBJECT_TYPE_DISPLAY_MODE_KHR";
	case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
		return "VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT";
	case VK_OBJECT_TYPE_CU_MODULE_NVX:
		return "VK_OBJECT_TYPE_CU_MODULE_NVX";
	case VK_OBJECT_TYPE_CU_FUNCTION_NVX:
		return "VK_OBJECT_TYPE_CU_FUNCTION_NVX";
	case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
		return "VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT";
	case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR:
		return "VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR";
	case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:
		return "VK_OBJECT_TYPE_VALIDATION_CACHE_EXT";
	case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV:
		return "VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV";
	default:
		break;
	}
	return "unknown";
}

void* allocateCallback(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	void* ptr = malloc(size); // TODO align this
	memset(ptr, 0, size);
	Logger::debug("[vulkan] Allocating ", (const char*)pUserData, " (bytes: ", size, ", alignment:", alignment, ", scope:", allocationScope, ")");
	return ptr;
}
void freeCallback(void* pUserData, void* pMemory)
{
	::free(pMemory);
	Logger::debug("[vulkan] Freeing ", (const char*)pUserData);
}
void internalAllocateCallback(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
	Logger::debug("[vulkan] Internal allocating ", (const char*)pUserData, " (bytes: ", size, ", type:", allocationType, ", scope:", allocationScope, ")");
}
void internalFreeCallback(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
	Logger::debug("[vulkan] Internal freeing ", (const char*)pUserData, " (bytes: ", size, ", type:", allocationType, ", scope:", allocationScope, ")");
}
void* reallocationCallback(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	Logger::debug("[vulkan] Reallocating ", (const char*)pUserData, " (bytes: ", size, ", alignment:", alignment, ", scope:", allocationScope, ")");
	return realloc(pOriginal, size);
}

VkAllocationCallbacks* vkGetAllocationCallbacks(VkObjectType objectType)
{
#if defined(VK_DEBUG_ALLOCATION)
	static std::map<VkObjectType, std::unique_ptr<VkAllocationCallbacks>> callbacks;
	auto it = callbacks.find(objectType);
	if (it == callbacks.end())
	{
		std::unique_ptr<VkAllocationCallbacks> callback = std::make_unique<VkAllocationCallbacks>();
		callback->pfnAllocation = allocateCallback;
		callback->pfnFree = freeCallback;
		callback->pfnInternalAllocation = internalAllocateCallback;
		callback->pfnInternalFree = internalFreeCallback;
		callback->pfnReallocation = reallocationCallback;
		callback->pUserData = (void*)vkGetObjectName(objectType);
		VkAllocationCallbacks* ptr = callback.get();
		auto it = callbacks.insert(std::make_pair(objectType, std::move(callback)));
		return ptr;
	}
	return it->second.get();
#else
	return nullptr; // Default allocator
#endif
}

};
};
#endif
