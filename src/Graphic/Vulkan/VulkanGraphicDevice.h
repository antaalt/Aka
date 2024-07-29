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
#include <Aka/OS/OS.h>

#define ENABLE_RENDERDOC_CAPTURE 1

#ifdef ENABLE_RENDERDOC_CAPTURE
struct RENDERDOC_API_1_6_0;
#endif

namespace aka {
namespace gfx {

class VulkanGraphicDevice : public GraphicDevice
{
public:
	VulkanGraphicDevice();
	~VulkanGraphicDevice();

	bool initialize(PlatformDevice* platform, const GraphicConfig& cfg) override;
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
	const Buffer* get(BufferHandle buffer) override;

	// Framebuffer
	FramebufferHandle createFramebuffer(const char* name, RenderPassHandle handle, const Attachment* attachments, uint32_t count, const Attachment* depth) override;
	void destroy(FramebufferHandle handle) override;
	void destroy(BackbufferHandle handle) override;
	BackbufferHandle createBackbuffer(RenderPassHandle handle) override;
	RenderPassHandle createBackbufferRenderPass(AttachmentLoadOp loadOp = AttachmentLoadOp::Clear, AttachmentStoreOp storeOp = AttachmentStoreOp::Store, ResourceAccessType initialLayout = ResourceAccessType::Undefined, ResourceAccessType finalLayout = ResourceAccessType::Present) override;
	FramebufferHandle get(BackbufferHandle handle, FrameHandle frame) override;
	const Framebuffer* get(FramebufferHandle handle) override;
	void getBackbufferSize(uint32_t& width, uint32_t& height) override;

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
	void beginMarker(QueueType queue, const char* name, const float* color) override;
	void endMarker(QueueType queue) override;

	// Frame command lists
	FrameIndex getFrameIndex(FrameHandle frame) override;
	CommandList* getCopyCommandList(FrameHandle frame) override;
	CommandList* getGraphicCommandList(FrameHandle frame) override;
	CommandList* getComputeCommandList(FrameHandle frame) override;
	CommandList* acquireCommandList(FrameHandle frame, QueueType queue) override;
	void release(FrameHandle frame, CommandList* cmd) override;

	// Frame
	FrameHandle frame() override;
	SwapchainStatus present(FrameHandle frame) override;
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
	VkDescriptorSetLayout getVkDescriptorSetLayout(const ShaderBindingState& state) { return m_context.getDescriptorSetLayout(state); }
	VkPipelineLayout getVkPipelineLayout(const VkDescriptorSetLayout* layouts, uint32_t layoutCount, const VkPushConstantRange* constants, uint32_t constantCount) { return m_context.getPipelineLayout(layouts, layoutCount, constants, constantCount); }
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) { return m_context.findMemoryType(typeFilter, properties); }
private:
#ifdef ENABLE_RENDERDOC_CAPTURE
	OS::Library m_renderDocLibrary;
	RENDERDOC_API_1_6_0* m_renderDocContext = nullptr;
	enum class RenderDocCaptureState {
		Idle,
		PendingCapture,
		Capturing,
	} m_captureState = RenderDocCaptureState::Idle;
#endif
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
	Pool<VulkanDescriptorSet> m_descriptorSetPool;
	Pool<VulkanDescriptorPool> m_descriptorPoolPool;
	Pool<VulkanFence> m_fencePool;
	Pool<VulkanCommandList> m_commandListPool;
};

};
};