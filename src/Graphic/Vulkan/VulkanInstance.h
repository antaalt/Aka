#pragma once

#include <Aka/Graphic/Pipeline.h>
#include <Aka/Graphic/Instance.h>
#include <Aka/Memory/Pool.h>

#if defined(AKA_USE_VULKAN)

#include <vulkan/vulkan.h>

#include "VulkanProgram.h"
#include "VulkanSwapchain.h"

namespace aka {
namespace gfx {

class VulkanInstance : public Instance
{
public:
	VulkanInstance();
	~VulkanInstance();

	void initialize() override;
	void shutdown() override;

	// Surface
	SurfaceHandle createSurface(const char* name, PlatformWindow* window) override;
	const Surface* get(SurfaceHandle handle) override;
	void destroy(SurfaceHandle surface) override;

	// Device
	GraphicDevice* pick(PhysicalDeviceFeatures features, gfx::SurfaceHandle surface) override;
	void destroy(GraphicDevice* device) override;

public:
	template <typename T, typename Base>
	T* getVk(ResourceHandle<Base> handle)
	{
		static_assert(std::is_base_of<Base, T>::value, "Handle is not valid for specified type.");
		return const_cast<T*>(reinterpret_cast<const T*>(get(handle)));
	}
	VkInstance getVkInstance() { return m_instance; }
	bool isValidationLayerEnabled();
	uint32_t getValidationLayerCount();
	const char* const* getValidationLayers();
	uint32_t getRequiredDeviceExtensionCount();
	const char* const* getRequiredDeviceExtensions();
private:
	VkInstance createInstance(const char** instanceExtensions, size_t instanceExtensionCount);
	uint32_t getPhysicalDeviceCount() const;
private:
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	Pool<VulkanSurface, 4> m_surfacePool; // Dont need more than 4.

};

}
}

#endif