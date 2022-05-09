#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Core/Config.h>
#include <Aka/OS/Logger.h>

#if defined(AKA_USE_VULKAN)

#include <vulkan/vulkan.h>

#include <type_traits>
#include <unordered_map>

#include "VulkanDebug.h"

namespace aka {
namespace gfx {

enum class VulkanRenderPassLayout
{
	Unknown,
	Backbuffer,
	Framebuffer,
};

struct VulkanInstance
{
	VkInstance instance;
};

struct VulkanDevice
{
	VkDevice device;
};

struct VulkanPhysicalDevice
{
	VkPhysicalDevice physicalDevice;
};

struct VulkanQueue
{
	uint32_t index;
	VkQueue queue;
};

struct VulkanSwapchain;
struct VulkanGraphicPipeline;
struct VulkanComputePipeline;
struct VulkanFramebuffer;
struct VulkanBuffer;

typedef bool (*PickPhysicalDeviceFunc)(const VkPhysicalDeviceProperties&, const VkPhysicalDeviceFeatures&);

struct VulkanContext
{
	void initialize(PlatformDevice* platform, const GraphicConfig& config);
	void shutdown();

	uint32_t getPhysicalDeviceCount() const;
	PhysicalDevice* getPhysicalDevice(uint32_t index);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	static VkFormat tovk(TextureFormat format);
	static VkFilter tovk(Filter filter);
	static VkIndexType tovk(IndexFormat format);
	static VkBufferUsageFlagBits tovk(BufferType type);

	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR surface;
	// swapchain
	VulkanQueue graphicQueue;
	VulkanQueue presentQueue;

	VkCommandPool commandPool;

public:
	struct VertexInputData {
		VkVertexInputBindingDescription bindings;
		VkVertexInputAttributeDescription attributes[VertexMaxAttributeCount];
	};
	struct ShaderInputData {
		VkDescriptorPool pool;
		VkDescriptorSetLayout layout;
		//VkPipelineLayout pipelineLayout;
	};
	VkRenderPass getRenderPass(const FramebufferState& fbDesc, VulkanRenderPassLayout layout);
	ShaderInputData getDescriptorLayout(const ShaderBindingState& bindingsDesc);
	VkPipelineLayout getPipelineLayout(const VkDescriptorSetLayout* layouts, uint32_t count);
	VertexInputData getVertexInputData(const VertexBindingState& verticesDesc);

private:
	std::unordered_map<FramebufferState, VkRenderPass> m_framebufferDesc;
	std::unordered_map<ShaderBindingState, ShaderInputData> m_bindingDesc;
	std::map<std::vector<VkDescriptorSetLayout>, VkPipelineLayout> m_pipelineLayout;
	std::unordered_map<VertexBindingState, VertexInputData> m_verticesDesc;

private:
	VkInstance createInstance(const char** instanceExtensions, size_t instanceExtensionCount);
	VkSurfaceKHR createSurface(PlatformDevice* platform);
	const char** getPlatformRequiredInstanceExtension(const PlatformDevice* platform, uint32_t* count);
	VkPhysicalDevice pickPhysicalDevice(PickPhysicalDeviceFunc isPhysicalDeviceSuitable);
	VkDevice createLogicalDevice(const char** deviceExtensions, size_t deviceExtensionCount);
};

};
};

#endif