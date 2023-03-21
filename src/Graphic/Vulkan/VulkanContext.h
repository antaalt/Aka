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
	static constexpr uint32_t invalidFamilyIndex = ~0;

	uint32_t familyIndex = invalidFamilyIndex; // family index
	uint32_t index = 0; // Index in family
	VkQueue queue = VK_NULL_HANDLE;
};

template <typename T, typename ...Args>
void setDebugName(VkDevice device, T handle, Args ...args)
{
	AKA_ASSERT(handle != VK_NULL_HANDLE, "Invalid handle");
	static_assert(VulkanTypeTrait<T>::debugType != VK_OBJECT_TYPE_UNKNOWN);
	String fmt_name = String::from(args...);
	VkDebugUtilsObjectNameInfoEXT info{};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	info.objectHandle = (uint64_t)handle;
	info.objectType = VulkanTypeTrait<T>::debugType;
	info.pObjectName = fmt_name.cstr();
	VK_CHECK_RESULT(vkSetDebugUtilsObjectNameEXT(device, &info));
}

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
	static VkMemoryPropertyFlags tovk(BufferUsage type);
	static VkImageLayout tovk(ResourceAccessType type, TextureFormat format);
	static VkAttachmentLoadOp tovk(AttachmentLoadOp loadOp);
	static VkAttachmentStoreOp tovk(AttachmentStoreOp storeOp);

	// TODO vk_ + private
	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR surface;
	// swapchain
	VulkanQueue queues[EnumCount<QueueType>()];
	VulkanQueue presentQueue;

	VkCommandPool commandPool[EnumCount<QueueType>()];
	VkDebugUtilsMessengerEXT debugMessenger;

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
	VkRenderPass getRenderPass(const RenderPassState& state);
	ShaderInputData getDescriptorLayout(const ShaderBindingState& bindingsDesc);
	VkPipelineLayout getPipelineLayout(const VkDescriptorSetLayout* layouts, uint32_t count);
	VertexInputData getVertexInputData(const VertexAttributeState& verticesDesc);

private:
	std::unordered_map<RenderPassState, VkRenderPass> m_renderPassState;
	std::unordered_map<ShaderBindingState, ShaderInputData> m_bindingDesc;
	std::map<std::vector<VkDescriptorSetLayout>, VkPipelineLayout> m_pipelineLayout;
	std::unordered_map<VertexAttributeState, VertexInputData> m_verticesDesc;

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