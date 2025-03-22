#pragma once

#include <Aka/Graphic/Pipeline.h>
#include <Aka/Graphic/Instance.h>
#include <Aka/Memory/Pool.h>
#include <Aka/OS/OS.h>

#if defined(AKA_USE_VULKAN)

#include <vulkan/vulkan.h>

#include "VulkanProgram.h"
#include "VulkanSwapchain.h"

#define ENABLE_RENDERDOC_CAPTURE 1

#ifdef ENABLE_RENDERDOC_CAPTURE
struct RENDERDOC_API_1_6_0;
enum class RenderDocCaptureState {
	Idle,
	PendingCapture,
	Capturing,
};
#endif

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
#ifdef ENABLE_RENDERDOC_CAPTURE
public:
	RENDERDOC_API_1_6_0* getRenderDocContext() { return m_renderDocContext; }
	RenderDocCaptureState getRenderDocCaptureState() { return m_captureState; }
	void setRenderDocCaptureState(RenderDocCaptureState state) { m_captureState = state; }
private:
	OS::Library m_renderDocLibrary;
	RENDERDOC_API_1_6_0* m_renderDocContext = nullptr;
	RenderDocCaptureState m_captureState = RenderDocCaptureState::Idle;
#endif
private:
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	Pool<VulkanSurface, 4> m_surfacePool; // Dont need more than 4.

};

}
}

#endif