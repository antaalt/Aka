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
	ShaderHandle createShader(ShaderType type, const void* data, size_t size) override;
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
		TextureFlag flags,
		const void* const* data
	) override;
	void upload(TextureHandle texture, const void* const* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
	void download(TextureHandle texture, void* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t mipLevel = 0, uint32_t layer = 0) override;
	void copy(TextureHandle lhs, TextureHandle rhs) override;
	void destroy(TextureHandle texture) override;
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
	FramebufferHandle createFramebuffer(const char* name, const Attachment* attachments, uint32_t count, const Attachment* depth) override;
	void destroy(FramebufferHandle handle) override;
	FramebufferHandle backbuffer(const Frame* frame) override;
	const Framebuffer* get(FramebufferHandle handle) override;

	// Pipeline
	GraphicPipelineHandle createGraphicPipeline(
		ProgramHandle program,
		PrimitiveType primitive,
		const FramebufferState& framebuffer,
		const VertexBindingState& vertices,
		const ViewportState& viewport,
		const DepthState& depth,
		const StencilState& stencil,
		const CullState& culling,
		const BlendState& blending,
		const FillState& fill
	) override;
	ComputePipelineHandle createComputePipeline(
		ProgramHandle program,
		uint32_t groupCountX,
		uint32_t groupCountY,
		uint32_t groupCountZ
	) override;
	void destroy(GraphicPipelineHandle handle) override;
	void destroy(ComputePipelineHandle handle) override;
	const GraphicPipeline* get(GraphicPipelineHandle handle) override;
	const ComputePipeline* get(ComputePipelineHandle handle) override;

	// Command list
	CommandList* acquireCommandList() override;
	void release(CommandList* cmd) override;
	void submit(CommandList** commands, uint32_t count, QueueType queue = QueueType::Default) override;
	void wait(QueueType queue) override;
	VkQueue getQueue(QueueType type);

	// Frame
	Frame* frame() override;
	void present(Frame* frame) override;
	void wait() override;

	// Tools
	void screenshot(void* data) override;


	template <typename T, typename Base>
	T* getVk(ResourceHandle<Base> handle)
	{
		static_assert(std::is_base_of<Base, T>::value, "Handle is not valid for specified type.");
		return const_cast<T*>(reinterpret_cast<const T*>(get(handle)));
	}

	VulkanContext& context() { return m_context; }
	VulkanSwapchain& swapchain() { return m_swapchain; }
private:
	friend struct VulkanSwapchain;
	// Context
	VulkanContext m_context;
	
	// Frame
	VulkanSwapchain m_swapchain;

	// Pools
	Pool<VulkanTexture> m_texturePool;
	Pool<VulkanSampler> m_samplerPool;
	Pool<VulkanBuffer> m_bufferPool;
	Pool<VulkanShader> m_shaderPool;
	Pool<VulkanProgram> m_programPool;
	Pool<VulkanFramebuffer> m_framebufferPool;
	Pool<VulkanGraphicPipeline> m_graphicPipelinePool;
	Pool<VulkanComputePipeline> m_computePipelinePool;
	Pool<VulkanDescriptorSet> m_descriptorPool;
	//Pool<VulkanCommandList> m_commandPool;
};

};
};