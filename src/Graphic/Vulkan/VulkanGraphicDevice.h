#pragma once

#include "VulkanContext.h"
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

#include <Aka/Memory/Pool.h>
#include <Aka/Core/Config.h>

namespace aka {
namespace gfx {

class VulkanGraphicDevice : public GraphicDevice
{
public:
	VulkanGraphicDevice(PlatformDevice* device, const GraphicConfig& cfg);
	~VulkanGraphicDevice();

	GraphicAPI api() const override;

	void name(const Resource* resource, const char* name) override;

	// Device
	uint32_t getPhysicalDeviceCount() override;
	const PhysicalDevice* getPhysicalDevice(uint32_t index) override;

	// Shaders
	ShaderHandle createShader(const char* name, ShaderType type, const void* data, size_t size) override;
	void destroy(ShaderHandle handle) override;
	const Shader* get(ShaderHandle handle) override;

	// Programs
	ProgramHandle createGraphicProgram(const char* name, ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, const ShaderBindingState* bindings, uint32_t bindingCounts) override;
	ProgramHandle createComputeProgram(const char* name, ShaderHandle compute, const ShaderBindingState* bindings, uint32_t bindingCounts) override;
	void destroy(ProgramHandle handle) override;
	const Program* get(ProgramHandle handle) override;

	// Descriptors
	DescriptorSetHandle createDescriptorSet(const char* name, const ShaderBindingState& bindings) override;
	void update(DescriptorSetHandle set, const DescriptorSetData& data) override;
	void destroy(DescriptorSetHandle set) override;
	const DescriptorSet* get(DescriptorSetHandle set) override;

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
	void copy(TextureHandle lhs, TextureHandle rhs) override;
	void destroy(TextureHandle texture) override;
	void transition(TextureHandle texture, ResourceAccessType src, ResourceAccessType dst) override;
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
	void upload(BufferHandle buffer, const void* data, uint32_t offset, uint32_t size) override;
	void download(BufferHandle buffer, void* data, uint32_t offset, uint32_t size) override;
	void* map(BufferHandle buffer, BufferMap map) override;
	void unmap(BufferHandle buffer) override;
	void destroy(BufferHandle buffer) override;
	const Buffer* get(BufferHandle buffer) override;

	// Framebuffer
	FramebufferHandle createFramebuffer(const char* name, RenderPassHandle handle, const Attachment* attachments, uint32_t count, const Attachment* depth) override;
	void destroy(FramebufferHandle handle) override;
	void destroy(BackbufferHandle handle) override;
	BackbufferHandle createBackbuffer(RenderPassHandle handle) override;
	RenderPassHandle createBackbufferRenderPass(AttachmentLoadOp loadOp = AttachmentLoadOp::Clear, AttachmentStoreOp storeOp = AttachmentStoreOp::Store, ResourceAccessType initialLayout = ResourceAccessType::Undefined, ResourceAccessType finalLayout = ResourceAccessType::Present) override;
	FramebufferHandle get(BackbufferHandle handle, Frame* frame) override;
	const Framebuffer* get(FramebufferHandle handle) override;

	// RenderPass
	RenderPassHandle createRenderPass(const char* name, const RenderPassState& state) override;
	void destroy(RenderPassHandle handle) override;
	const RenderPass* get(RenderPassHandle handle) override;

	// Pipeline
	GraphicPipelineHandle createGraphicPipeline(
		const char* name,
		ProgramHandle program,
		PrimitiveType primitive,
		const RenderPassState& renderPass,
		const VertexAttributeState& vertices,
		const ViewportState& viewport,
		const DepthState& depth,
		const StencilState& stencil,
		const CullState& culling,
		const BlendState& blending,
		const FillState& fill
	) override;
	ComputePipelineHandle createComputePipeline(
		const char* name,
		ProgramHandle program
	) override;
	void destroy(GraphicPipelineHandle handle) override;
	void destroy(ComputePipelineHandle handle) override;
	const GraphicPipeline* get(GraphicPipelineHandle handle) override;
	const ComputePipeline* get(ComputePipelineHandle handle) override;

	// Fence
	FenceHandle createFence(const char* name) override;
	void destroy(FenceHandle handle) override;
	const Fence* get(FenceHandle handle) override;
	void wait(FenceHandle handle, FenceValue waitValue) override;
	void signal(FenceHandle handle, FenceValue value) override;
	FenceValue read(FenceHandle handle) override;

	// Command list
	CommandList* acquireCommandList(QueueType queue) override;
	void release(CommandList* cmd) override;
	void submit(CommandList* command, FenceHandle handle = FenceHandle::null, FenceValue waitValue = 0U, FenceValue signalValue = 0U) override;
	void wait(QueueType queue) override;

	// Frame command lists
	CommandList* getCopyCommandList(Frame* frame) override;
	CommandList* getGraphicCommandList(Frame* frame) override;
	CommandList* getComputeCommandList(Frame* frame) override;

	// Frame
	Frame* frame() override;
	SwapchainStatus present(Frame* frame) override;
	void wait() override;

	// Tools
	void screenshot(void* data) override;


	template <typename T, typename Base>
	T* getVk(ResourceHandle<Base> handle)
	{
		static_assert(std::is_base_of<Base, T>::value, "Handle is not valid for specified type.");
		return const_cast<T*>(reinterpret_cast<const T*>(get(handle)));
	}

public:
	uint32_t getSwapchainImageCount() { return m_swapchain.getImageCount(); }
public:
	VkInstance getVkInstance() { return m_context.instance; }
	VkDevice getVkDevice() { return m_context.device; }
	VkPhysicalDevice getVkPhysicalDevice() { return m_context.physicalDevice; }
	VkCommandPool getVkCommandPool(QueueType queue) { return m_context.commandPool[EnumToIndex(queue)]; }
	VkQueue getVkQueue(QueueType type);
	VkQueue getVkPresentQueue();
	uint32_t getVkQueueIndex(QueueType queue);
	uint32_t getVkPresentQueueIndex();
	VkSurfaceKHR getVkSurface() { return m_context.surface; }
	VkRenderPass getVkRenderPass(const RenderPassState& state) { return m_context.getRenderPass(state); }
	VkDescriptorSetLayout getVkDescriptorSetLayout(const ShaderBindingState& state) { return m_context.getDescriptorLayout(state).layout; }
	VkDescriptorPool getVkDescriptorPool(const ShaderBindingState& state) { return m_context.getDescriptorLayout(state).pool; }
	VkPipelineLayout getVkPipelineLayout(const VkDescriptorSetLayout* layouts, uint32_t count) { return m_context.getPipelineLayout(layouts, count); }
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) { return m_context.findMemoryType(typeFilter, properties); }
private:
	friend class VulkanSwapchain;
	VulkanContext m_context;
	VulkanSwapchain m_swapchain;
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
	Pool<VulkanDescriptorSet> m_descriptorPool;
	Pool<VulkanFence> m_fencePool;
	Pool<VulkanCommandList> m_commandListPool;
};

};
};