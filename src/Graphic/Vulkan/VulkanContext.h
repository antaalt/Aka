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

template <>
struct std::hash<VkPushConstantRange>
{
	size_t operator()(const VkPushConstantRange& data) const
	{
		size_t hash = 0;
		aka::hash::combine(hash, data.offset);
		aka::hash::combine(hash, data.size);
		aka::hash::combine(hash, data.stageFlags);
		return hash;
	}
};
using PipelineLayoutKey = std::pair<aka::Vector<VkDescriptorSetLayout>, aka::Vector<VkPushConstantRange>>;

template <>
struct std::less<VkPushConstantRange>
{
	bool operator()(const VkPushConstantRange& lhs, const VkPushConstantRange& rhs) const
	{
		if (lhs.offset < rhs.offset) return true;
		else if (lhs.offset > rhs.offset) return false;

		if (lhs.size < rhs.size) return true;
		else if (lhs.size > rhs.size) return false;

		if (lhs.stageFlags < rhs.stageFlags) return true;
		else if (lhs.stageFlags > rhs.stageFlags) return false;

		return false; // equal
	}
};

struct PipelineLayoutKeyFunctor
{
	bool operator()(const PipelineLayoutKey& left, const PipelineLayoutKey& right) const
	{
		if (left.first != right.first) return false;
		for (size_t i = 0; i < right.second.size(); i++)
		{
			if (right.second[i].offset != left.second[i].offset) return false;
			if (right.second[i].size != left.second[i].size) return false;
			if (right.second[i].stageFlags != left.second[i].stageFlags) return false;
		}
		return true;
	}
	size_t operator()(const PipelineLayoutKey& data) const
	{
		size_t hash = 0;
		for (VkDescriptorSetLayout layout : data.first)
		{
			aka::hash::combine(hash, layout);
		}
		for (VkPushConstantRange range : data.second)
		{
			aka::hash::combine(hash, range.offset);
			aka::hash::combine(hash, range.size);
			aka::hash::combine(hash, range.stageFlags);
		}
		return hash;
	}
};


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

struct VulkanContext
{
	bool initialize(PlatformDevice* platform, const GraphicConfig& config);
	void shutdown();

	uint32_t getPhysicalDeviceCount() const;
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
	static VkDescriptorType tovk(ShaderBindingType type);
	static VkShaderStageFlags tovk(ShaderMask shaderType);

	// TODO vk_ + private
	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	PhysicalDeviceFeatures physicalDeviceFeatures;
	PhysicalDeviceLimits physicalDeviceLimits;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	// swapchain
	VulkanQueue queues[EnumCount<QueueType>()];
	VulkanQueue presentQueue;

	VkCommandPool commandPool[EnumCount<QueueType>()];
	VkDebugUtilsMessengerEXT debugMessenger;

	PlatformDevice* getPlatform() { return m_platform; }

private:
	PlatformDevice* m_platform;

public:
	VkRenderPass getRenderPass(const RenderPassState& state);
	VkDescriptorSetLayout getDescriptorSetLayout(const ShaderBindingState& bindingsDesc);
	VkPipelineLayout getPipelineLayout(const VkDescriptorSetLayout* layouts, uint32_t layoutCount, const VkPushConstantRange* constants, uint32_t constantCount);

private:
	std::unordered_map<RenderPassState, VkRenderPass> m_renderPassState;
	std::unordered_map<ShaderBindingState, VkDescriptorSetLayout> m_descriptorSetLayouts;
	std::unordered_map<PipelineLayoutKey, VkPipelineLayout, PipelineLayoutKeyFunctor, PipelineLayoutKeyFunctor> m_pipelineLayout; // Should not cache this, heavy to cache, and not that useful...
private:
	VkInstance createInstance(const char** instanceExtensions, size_t instanceExtensionCount);
	VkSurfaceKHR createSurface(PlatformDevice* platform);
	const char** getPlatformRequiredInstanceExtension(const PlatformDevice* platform, uint32_t* count);
	std::tuple<VkPhysicalDevice, PhysicalDeviceFeatures, PhysicalDeviceLimits> pickPhysicalDevice(PhysicalDeviceFeatures _requestedFeatures);
	VkDevice createLogicalDevice(const char* const* deviceExtensions, size_t deviceExtensionCount);
};

};
};

#endif