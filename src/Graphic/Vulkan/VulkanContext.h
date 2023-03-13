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
	uint32_t index;
	VkQueue queue;
};


template <typename T> struct VulkanTypeTrait { static const VkObjectType debugType = VK_OBJECT_TYPE_UNKNOWN; };
template <> struct VulkanTypeTrait<VkImage> { static const VkObjectType debugType = VK_OBJECT_TYPE_IMAGE; };
template <> struct VulkanTypeTrait<VkDeviceMemory> { static const VkObjectType debugType = VK_OBJECT_TYPE_DEVICE_MEMORY; };
template <> struct VulkanTypeTrait<VkImageView> { static const VkObjectType debugType = VK_OBJECT_TYPE_IMAGE_VIEW; };
template <> struct VulkanTypeTrait<VkBuffer> { static const VkObjectType debugType = VK_OBJECT_TYPE_BUFFER; };
template <> struct VulkanTypeTrait<VkCommandPool> { static const VkObjectType debugType = VK_OBJECT_TYPE_COMMAND_POOL; };
template <> struct VulkanTypeTrait<VkRenderPass> { static const VkObjectType debugType = VK_OBJECT_TYPE_RENDER_PASS; };
template <> struct VulkanTypeTrait<VkSampler> { static const VkObjectType debugType = VK_OBJECT_TYPE_SAMPLER; };
template <> struct VulkanTypeTrait<VkShaderModule> { static const VkObjectType debugType = VK_OBJECT_TYPE_SHADER_MODULE; };
template <> struct VulkanTypeTrait<VkDescriptorSet> { static const VkObjectType debugType = VK_OBJECT_TYPE_DESCRIPTOR_SET; };
template <> struct VulkanTypeTrait<VkDescriptorSetLayout> { static const VkObjectType debugType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; };
template <> struct VulkanTypeTrait<VkDescriptorPool> { static const VkObjectType debugType = VK_OBJECT_TYPE_DESCRIPTOR_POOL; };

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
	static VkImageLayout tovk(ResourceAccessType type, TextureFormat format);
	static VkAttachmentLoadOp tovk(AttachmentLoadOp loadOp);
	static VkAttachmentStoreOp tovk(AttachmentStoreOp storeOp);

	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR surface;
	// swapchain
	VulkanQueue graphicQueue;
	VulkanQueue presentQueue;

	VkCommandPool commandPool;
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
	VertexInputData getVertexInputData(const VertexBindingState& verticesDesc);

private:
	std::unordered_map<RenderPassState, VkRenderPass> m_renderPassState;
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