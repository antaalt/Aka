#pragma once

#if defined(AKA_USE_VULKAN)


#include <vulkan/vulkan.h>

#define VK_DEBUG_ALLOCATION

#define VK_CHECK_RESULT(result)				\
{											\
	VkResult res = (result);				\
	if (VK_SUCCESS != res) {				\
		char buffer[256];					\
		snprintf(							\
			buffer,							\
			256,							\
			"%s (%s at %s:%d)",				\
			aka::gfx::vkGetErrorString(res),\
			AKA_STRINGIFY(result),			\
			__FILE__,						\
			__LINE__						\
		);									\
		::aka::Logger::error(buffer);       \
		AKA_DEBUG_BREAK;                    \
	}										\
}

namespace aka {
namespace gfx {

// Get a vulkan error string
const char* vkGetErrorString(VkResult result);
// Get an object name
const char* vkGetObjectName(VkObjectType objectType);
// Get allocation callback for given object type
VkAllocationCallbacks* vkGetAllocationCallbacks(VkObjectType objectType);

};
};

#endif