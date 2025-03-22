#pragma once

#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanProgram.h"
#include "VulkanPipeline.h"
#include "VulkanFramebuffer.h"
#include "VulkanCommandList.h"
#include "VulkanSwapchain.h"
#include "VulkanSampler.h"
#include "VulkanRenderPass.h"
#include "VulkanFence.h"
#include "VulkanDebug.h"
#include "VulkanCommon.hpp"

#include <Aka/Memory/Pool.h>
#include <Aka/Core/Config.h>
#include <Aka/OS/OS.h>


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
using PipelineLayoutKey = std::pair<aka::gfx::Vector<VkDescriptorSetLayout>, aka::gfx::Vector<VkPushConstantRange>>;

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

struct VulkanQueue
{
	static constexpr uint32_t invalidFamilyIndex = ~0;

	uint32_t familyIndex = invalidFamilyIndex; // family index
	uint32_t index = 0; // Index in family
	VkQueue queue = VK_NULL_HANDLE;
};


class VulkanGraphicDevice : public GraphicDevice
{
public:
	VulkanGraphicDevice(VulkanInstance* instance, VkPhysicalDevice _device, PhysicalDeviceFeatures features, const PhysicalDeviceLimits& limits);
	~VulkanGraphicDevice();

	bool initialize(gfx::SurfaceHandle surface) override;
	void shutdown() override;

	GraphicAPI getApi() const override;
	PhysicalDeviceFeatures getFeatures() const override;
	bool hasFeatures(PhysicalDeviceFeatures _features) const override;
	PhysicalDeviceLimits getLimits() const override;

	// Shaders
	ShaderHandle createShader(const char* name, ShaderType type, const char* entryPoint, const void* data, size_t size) override;
	void destroy(ShaderHandle handle) override;
	const Shader* get(ShaderHandle handle) override;

	// Programs
	ProgramHandle createVertexProgram(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderPipelineLayout& layout) override;
	ProgramHandle createMeshProgram(const char* name, ShaderHandle task, ShaderHandle mesh, ShaderHandle fragment, const ShaderPipelineLayout& layout) override;
	ProgramHandle createComputeProgram(const char* name, ShaderHandle compute, const ShaderPipelineLayout& layout) override;
	void replace(ProgramHandle oldProgram, ProgramHandle newProgram) override;
	void destroy(ProgramHandle handle) override;
	const Program* get(ProgramHandle handle) override;

	// Descriptors
	DescriptorSetHandle allocateDescriptorSet(const char* name, const ShaderBindingState& bindings, DescriptorPoolHandle pool) override;
	void update(DescriptorSetHandle set, const DescriptorUpdate* update, size_t size) override;
	void free(DescriptorSetHandle set) override;
	const DescriptorSet* get(DescriptorSetHandle set) override;

	DescriptorPoolHandle createDescriptorPool(const char* name, const ShaderBindingState& bindings, uint32_t size) override;
	const DescriptorPool* get(DescriptorPoolHandle handle) override;
	void destroy(DescriptorPoolHandle pool) override;

	// Textures
	TextureHandle createTexture(
		const char* name,
		uint32_t width, uint32_t height, uint32_t depth,
		TextureType type,
		uint32_t levels, uint32_t layers,
		TextureFormat format,
		TextureUsage flags,
		const void* const* data
	) override;
	void upload(TextureHandle texture, const void* const* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
	void download(TextureHandle texture, void* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t mipLevel = 0, uint32_t layer = 0) override;
	void copy(TextureHandle src, TextureHandle dst) override;
	void destroy(TextureHandle texture) override;
	void transition(TextureHandle texture, ResourceAccessType src, ResourceAccessType dst) override;
	void transfer(TextureHandle texture, QueueType srcQueue, QueueType dstQueue, ResourceAccessType src, ResourceAccessType dst) override;
	const Texture* get(TextureHandle texture) override;

	// Sampler
	SamplerHandle createSampler(
		const char* name,
		Filter filterMin,
		Filter filterMag,
		SamplerMipMapMode mipmapMode,
		SamplerAddressMode wrapU,
		SamplerAddressMode wrapV,
		SamplerAddressMode wrapW,
		float anisotropy
	) override;
	void destroy(SamplerHandle sampler) override;
	const Sampler* get(SamplerHandle handle) override;

	// Buffer
	BufferHandle createBuffer(const char* name, BufferType type, uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data = nullptr) override;
	void upload(BufferHandle buffer, const void* data, size_t offset, size_t size) override;
	void download(BufferHandle buffer, void* data, size_t offset, size_t size) override;
	void copy(BufferHandle src, BufferHandle dst) override;
	void* map(BufferHandle buffer, BufferMap map) override;
	void unmap(BufferHandle buffer) override;
	void destroy(BufferHandle buffer) override;
	void transition(BufferHandle buffer, ResourceAccessType src, ResourceAccessType dst) override;
	void transfer(BufferHandle buffer, QueueType srcQueue, QueueType dstQueue, ResourceAccessType src, ResourceAccessType dst) override;
	const Buffer* get(BufferHandle buffer) override;

	// Framebuffer
	FramebufferHandle createFramebuffer(const char* name, RenderPassHandle handle, const Attachment* attachments, uint32_t count = 1, const Attachment* depth = nullptr) override;
	void destroy(FramebufferHandle handle) override;
	const Framebuffer* get(FramebufferHandle handle) override;

	// Swapchain
	SwapchainHandle createSwapchain(const char* name, SurfaceHandle surface, uint32_t width, uint32_t height, TextureFormat format, SwapchainMode mode, SwapchainType type) override;
	void destroy(SwapchainHandle backbuffer) override;
	const Swapchain* get(SwapchainHandle swapchain) override;
	BackbufferHandle createBackbuffer(const char* _name, SwapchainHandle swapchainHandle, RenderPassHandle _handle, const Attachment* _additionalAttachments = nullptr, uint32_t _count = 0, const Attachment* _depth = nullptr) override;
	void destroy(SwapchainHandle _swapchainHandle, BackbufferHandle handle) override;
	FramebufferHandle get(BackbufferHandle handle, SwapchainHandle _swapchainHandle, FrameHandle frame) override;
	SwapchainExtent getSwapchainExtent(SwapchainHandle handle) override;
	TextureFormat getSwapchainFormat(SwapchainHandle handle) override;
	uint32_t getSwapchainImageCount(SwapchainHandle handle) override;
	void resize(SwapchainHandle handle, uint32_t width, uint32_t height, TextureFormat format, SwapchainMode mode, SwapchainType type) override;

	// RenderPass
	RenderPassHandle createRenderPass(const char* name, const RenderPassState& state) override;
	void destroy(RenderPassHandle handle) override;
	const RenderPass* get(RenderPassHandle handle) override;

	// Pipeline
	GraphicPipelineHandle createGraphicPipeline(
		const char* name,
		ProgramHandle program,
		PrimitiveType primitive,
		const ShaderPipelineLayout& layout,
		const RenderPassState& renderPass,
		const VertexState& vertices,
		const ViewportState& viewport,
		const DepthState& depth,
		const StencilState& stencil,
		const CullState& culling,
		const BlendState& blending,
		const FillState& fill
	) override;
	ComputePipelineHandle createComputePipeline(
		const char* name,
		ProgramHandle program,
		const ShaderPipelineLayout& layout
	) override;
	void destroy(GraphicPipelineHandle handle) override;
	void destroy(ComputePipelineHandle handle) override;
	const GraphicPipeline* get(GraphicPipelineHandle handle) override;
	const ComputePipeline* get(ComputePipelineHandle handle) override;

	// Fence
	FenceHandle createFence(const char* name, FenceValue value) override;
	void destroy(FenceHandle handle) override;
	const Fence* get(FenceHandle handle) override;
	void wait(FenceHandle handle, FenceValue waitValue) override;
	void signal(FenceHandle handle, FenceValue value) override;
	FenceValue read(FenceHandle handle) override;

	// Command list
	CommandEncoder* acquireCommandEncoder(QueueType queue) override;
	void execute(const char* _name, std::function<void(CommandList&)> callback, QueueType queue, bool async = true) override;
	void executeVk(const char* _name, std::function<void(VulkanCommandList&)> callback, QueueType queue, bool async = true);
	void release(CommandEncoder* cmd) override;
	void submit(CommandEncoder* command, FenceHandle handle = FenceHandle::null, FenceValue waitValue = InvalidFenceValue, FenceValue signalValue = InvalidFenceValue) override;
	void wait(QueueType queue) override;
	void beginMarker(QueueType queue, const char* name, const float* color) override;
	void endMarker(QueueType queue) override;

	// Frame command lists
	FrameIndex getFrameIndex(FrameHandle frame) override;
	CommandList* getCopyCommandList(FrameHandle frame) override;
	CommandList* getGraphicCommandList(FrameHandle frame) override;
	CommandList* getComputeCommandList(FrameHandle frame) override;
	CommandEncoder* acquireCommandEncoder(FrameHandle frame, QueueType queue) override;
	void release(FrameHandle frame, CommandEncoder* cmd) override;

	// Frame
	const Frame* get(FrameHandle handle) override;
	FrameHandle frame(SwapchainHandle handle) override;
	SwapchainStatus present(SwapchainHandle handle, FrameHandle frame) override;
	void wait() override;

	// Tools
	void screenshot(void* data) override;
	void capture() override;


	template <typename T, typename Base>
	T* getVk(ResourceHandle<Base> handle)
	{
		static_assert(std::is_base_of<Base, T>::value, "Handle is not valid for specified type.");
		return const_cast<T*>(reinterpret_cast<const T*>(get(handle)));
	}

public:
	VkInstance getVkInstance();
	VkDevice getVkDevice() { return m_device; }
	VkPhysicalDevice getVkPhysicalDevice() { return m_physicalDevice; }
	VkCommandPool getVkCommandPool(QueueType queue) { return m_commandPool[EnumToIndex(queue)]; }
	VkQueue getVkQueue(QueueType type);
	VkQueue getVkPresentQueue();
	uint32_t getVkQueueIndex(QueueType queue);
	uint32_t getVkPresentQueueIndex();
	VkRenderPass getVkRenderPass(const RenderPassState& state);
	VkDescriptorSetLayout getVkDescriptorSetLayout(const ShaderBindingState& state);
	VkPipelineLayout getVkPipelineLayout(const VkDescriptorSetLayout* layouts, uint32_t layoutCount, const VkPushConstantRange* constants, uint32_t constantCount);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
private:
private:
	std::unordered_map<RenderPassState, VkRenderPass> m_renderPassState;
	std::unordered_map<ShaderBindingState, VkDescriptorSetLayout> m_descriptorSetLayouts;
	std::unordered_map<PipelineLayoutKey, VkPipelineLayout, PipelineLayoutKeyFunctor, PipelineLayoutKeyFunctor> m_pipelineLayout; // Should not cache this, heavy to cache, and not that useful...
private:
	VkDevice createLogicalDevice(const char* const* deviceExtensions, size_t deviceExtensionCount, gfx::SurfaceHandle surface);
private:
	friend class VulkanSwapchain;
	VulkanInstance* m_instance;
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	PhysicalDeviceFeatures m_physicalDeviceFeatures;
	PhysicalDeviceLimits m_physicalDeviceLimits;
	// swapchain
	VulkanQueue m_queues[EnumCount<QueueType>()];
	VulkanQueue m_presentQueue;

	VkCommandPool m_commandPool[EnumCount<QueueType>()];
	//VulkanSwapchain m_swapchain;
	// Improve upload / download with persistent staging memory
	const uint32_t m_stagingUploadHeapSize = 1 << 23; // 8 Mo
	VkDeviceMemory m_stagingUploadMemory = VK_NULL_HANDLE;
	VkBuffer m_stagingUploadBuffer = VK_NULL_HANDLE;
	const uint32_t m_stagingDownloadHeapSize = 1 << 23; // 8 Mo
	VkDeviceMemory m_stagingDownloadMemory = VK_NULL_HANDLE;
	VkBuffer m_stagingDownloadBuffer = VK_NULL_HANDLE;
	// Delayed command encoder release
	FenceHandle m_copyFenceHandle = FenceHandle::null;
	FenceValue m_copyFenceCounter = 0;
	Vector<VulkanCommandEncoder*> m_commandEncoderToRelease;
private: // Pools
	Pool<VulkanTexture> m_texturePool;
	Pool<VulkanSampler> m_samplerPool;
	Pool<VulkanBuffer> m_bufferPool;
	Pool<VulkanShader> m_shaderPool;
	Pool<VulkanProgram> m_programPool;
	Pool<VulkanFramebuffer> m_framebufferPool;
	Pool<VulkanRenderPass> m_renderPassPool;
	Pool<VulkanGraphicPipeline> m_graphicPipelinePool;
	Pool<VulkanComputePipeline> m_computePipelinePool;
	Pool<VulkanDescriptorSet> m_descriptorSetPool;
	Pool<VulkanDescriptorPool> m_descriptorPoolPool;
	Pool<VulkanFence> m_fencePool;
	Pool<VulkanSwapchain> m_swapchainPool;
	Pool<VulkanCommandEncoder> m_commandEncoderPool;
};

};
};