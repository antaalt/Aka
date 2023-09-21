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
		aka::hash(hash, &data.offset, sizeof(uint32_t));
		aka::hash(hash, &data.size, sizeof(uint32_t));
		aka::hash(hash, &data.stageFlags, sizeof(VkShaderStageFlags));
		return hash;
	}
};
using PipelineLayoutKey = std::pair<std::vector<VkDescriptorSetLayout>, std::vector<VkPushConstantRange>>;

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
		for (size_t i = 0; i < data.first.size(); i++)
		{
			aka::hash((uintptr_t)data.first[i]);
		}
		for (size_t i = 0; i < data.second.size(); i++)
		{
			aka::hash(hash, &data.second[i].offset, sizeof(uint32_t));
			aka::hash(hash, &data.second[i].size, sizeof(uint32_t));
			aka::hash(hash, &data.second[i].stageFlags, sizeof(VkShaderStageFlags));
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
	static VkDescriptorType tovk(ShaderBindingType type);
	static VkShaderStageFlags tovk(ShaderMask shaderType);

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

	PlatformDevice* getPlatform() { return m_platform; }

private:
	PlatformDevice* m_platform;

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
	VkPipelineLayout getPipelineLayout(const VkDescriptorSetLayout* layouts, uint32_t layoutCount, const VkPushConstantRange* constants, uint32_t constantCount);
	VertexInputData getVertexInputData(const VertexAttributeState& verticesDesc);

private:
	std::unordered_map<RenderPassState, VkRenderPass> m_renderPassState;
	std::unordered_map<ShaderBindingState, ShaderInputData> m_bindingDesc;
	std::unordered_map<PipelineLayoutKey, VkPipelineLayout, PipelineLayoutKeyFunctor, PipelineLayoutKeyFunctor> m_pipelineLayout;
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