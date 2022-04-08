#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Core/Config.h>
#include <Aka/OS/Logger.h>

#if defined(AKA_USE_VULKAN)

#include <vulkan/vulkan.h>

#include <type_traits>

#define VK_CHECK_RESULT(result)				\
{											\
	VkResult res = (result);				\
	if (VK_SUCCESS != res) {				\
		char buffer[256];					\
		snprintf(							\
			buffer,							\
			256,							\
			"%s (%s at %s:%d)",				\
			vkGetErrorString(res),	        \
			AKA_STRINGIFY(result),			\
			__FILE__,						\
			__LINE__						\
		);									\
		::aka::Logger::error(buffer);       \
		AKA_DEBUG_BREAK;                    \
	}										\
}


const char* vkGetErrorString(VkResult result);

namespace aka {

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
struct VulkanPipeline;
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
		VkVertexInputAttributeDescription attributes[VertexBindingState::MaxAttributes];
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
	struct CacheComparator {
		// TODO use hash instead ?
		bool operator()(const FramebufferState& lhs, const FramebufferState& rhs) const {
			if (lhs.count < rhs.count) return true;
			else if (lhs.count > rhs.count) return false;
			for (uint32_t i = 0; i < lhs.count; i++)
			{
				if (lhs.colors[i].format < rhs.colors[i].format) return true;
				else if (lhs.colors[i].format > rhs.colors[i].format) return false;
				if (lhs.colors[i].loadOp < rhs.colors[i].loadOp) return true;
				else if (lhs.colors[i].loadOp > rhs.colors[i].loadOp) return false;
			}
			if (lhs.depth.format < rhs.depth.format) return true;
			else if (lhs.depth.format > rhs.depth.format) return false;
			if (lhs.depth.loadOp < rhs.depth.loadOp) return true;
			else if (lhs.depth.loadOp > rhs.depth.loadOp) return false;
			return false; // equal
		}
		bool operator()(const ShaderBindingState& lhs, const ShaderBindingState& rhs) const {
			if (lhs.count < rhs.count) return true;
			else if (lhs.count > rhs.count) return false;
			for (uint32_t i = 0; i < lhs.count; i++)
			{
				if (lhs.bindings[i].count < rhs.bindings[i].count) return true;
				else if (lhs.bindings[i].count > rhs.bindings[i].count) return false;
				if (lhs.bindings[i].shaderType < rhs.bindings[i].shaderType) return true;
				else if (lhs.bindings[i].shaderType > rhs.bindings[i].shaderType) return false;
				if (lhs.bindings[i].type < rhs.bindings[i].type) return true;
				else if (lhs.bindings[i].type > rhs.bindings[i].type) return false;
			}
			return false; // equal
		}
		bool operator()(const VertexBindingState& lhs, const VertexBindingState& rhs) const {

			if (lhs.count < rhs.count) return true;
			else if (lhs.count > rhs.count) return false;
			for (uint32_t i = 0; i < lhs.count; i++)
			{
				if (lhs.attributes[i].semantic < rhs.attributes[i].semantic) return true;
				else if (lhs.attributes[i].semantic > rhs.attributes[i].semantic) return false;
				if (lhs.attributes[i].format < rhs.attributes[i].format) return true;
				else if (lhs.attributes[i].format > rhs.attributes[i].format) return false;
				if (lhs.attributes[i].type < rhs.attributes[i].type) return true;
				else if (lhs.attributes[i].type > rhs.attributes[i].type) return false;
				if (lhs.offsets[i] < rhs.offsets[i]) return true;
				else if (lhs.offsets[i] > rhs.offsets[i]) return false;
			}
			return false; // equal
		}
	};
	std::map<FramebufferState, VkRenderPass, CacheComparator> m_framebufferDesc;
	std::map<ShaderBindingState, ShaderInputData, CacheComparator> m_bindingDesc;
	std::map<std::vector<VkDescriptorSetLayout>, VkPipelineLayout> m_pipelineLayout;
	std::map<VertexBindingState, VertexInputData, CacheComparator> m_verticesDesc;

private:
	VkInstance createInstance(const char** instanceExtensions, size_t instanceExtensionCount);
	VkSurfaceKHR createSurface(PlatformDevice* platform);
	const char** getPlatformRequiredInstanceExtension(const PlatformDevice* platform, uint32_t* count);
	VkPhysicalDevice pickPhysicalDevice(PickPhysicalDeviceFunc isPhysicalDeviceSuitable);
	VkDevice createLogicalDevice(const char** deviceExtensions, size_t deviceExtensionCount);
};

};

#endif