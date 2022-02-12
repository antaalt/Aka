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

namespace aka {

class VulkanGraphicDevice : public GraphicDevice
{
public:
	VulkanGraphicDevice(PlatformDevice* device, const GraphicConfig& cfg);
	~VulkanGraphicDevice();

	GraphicAPI api() const override;

	// Device
	uint32_t getPhysicalDeviceCount() override;
	PhysicalDevice* getPhysicalDevice(uint32_t index) override;

	// Shaders
	Shader* compile(ShaderType type, const uint8_t* data, size_t size) override;
	void destroy(Shader* handle) override;

	// Programs
	Program* createProgram(Shader* vertex, Shader* fragment, Shader* geometry, const ShaderBindingState& bindings) override;
	void destroy(Program* handle) override;
	Material* createMaterial(Program* program) override;
	void destroy(Material* material) override;

	// Textures
	Texture* createTexture(
		uint32_t width, uint32_t height, uint32_t depth,
		TextureType type,
		uint32_t levels, uint32_t layers,
		TextureFormat format,
		TextureFlag flags,
		const void* const* data
	) override;
	void upload(const Texture* texture, const void*const* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
	void download(const Texture* texture, void* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t mipLevel = 0, uint32_t layer = 0) override;
	void copy(const Texture* lhs, const Texture* rhs) override;
	void destroy(Texture* texture) override;

	// Sampler
	Sampler* createSampler(
		Filter filterMin, 
		Filter filterMag, 
		SamplerMipMapMode mipmapMode, 
		SamplerAddressMode wrapU, 
		SamplerAddressMode wrapV, 
		SamplerAddressMode wrapW, 
		float anisotropy
	) override;
	void destroy(Sampler* sampler) override;

	// Buffer
	Buffer* createBuffer(BufferType type, uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data = nullptr) override;
	void upload(const Buffer* buffer, const void* data, uint32_t offset, uint32_t size) override;
	void download(const Buffer* buffer, void* data, uint32_t offset, uint32_t size) override;
	void* map(Buffer* buffer, BufferMap map) override;
	void unmap(Buffer* buffer) override;
	void destroy(Buffer* buffer) override;

	// Framebuffer
	Framebuffer* createFramebuffer(const Attachment* attachments, uint32_t count, const Attachment* depth) override;
	void destroy(Framebuffer* handle) override;
	Framebuffer* backbuffer(Frame* frame) override;

	// Pipeline
	Pipeline* createPipeline(
		Shader** shaders,
		uint32_t shaderCount,
		PrimitiveType primitive,
		const FramebufferState& framebuffer,
		const VertexBindingState& vertices,
		const ShaderBindingState& bindings,
		const ViewportState& viewport,
		const DepthState& depth,
		const StencilState& stencil,
		const CullState& culling,
		const BlendState& blending,
		const FillState& fill
	) override;
	void destroy(Pipeline* handle) override;

	// Command list
	CommandList* acquireCommandList() override;
	void release(CommandList* cmd) override;
	void submit(CommandList** commands, uint32_t count, QueueType queue = QueueType::Default) override;
	void wait(QueueType queue) override;
	VkQueue getQueue(QueueType type);

	// Frame
	Frame* frame() override;
	void present(Frame* frame) override;

	// Tools
	void screenshot(void* data) override;

	VulkanContext& context() { return m_context; }
	VulkanSwapchain& swapchain() { return m_swapchain; }
private:
	VulkanTexture* makeTexture(
		uint32_t width, uint32_t height, uint32_t depth,
		uint32_t levels, uint32_t layers,
		TextureFormat format,
		TextureType type,
		TextureFlag flags,
		VkImage image,
		VkImageView view,
		VkDeviceMemory memory,
		VkImageLayout layout
	);

	VulkanBuffer* makeBuffer(
		BufferType type,
		uint32_t size,
		BufferUsage usage,
		BufferCPUAccess access,
		VkBuffer buffer,
		VkDeviceMemory memory
	);

	VulkanShader* makeShader(
		ShaderType type,
		VkShaderModule module
	);
private:
	friend struct VulkanSwapchain;
	// Context
	VulkanContext m_context;
	
	// Frame
	VulkanSwapchain m_swapchain;

	// Pools
	GraphicPool<VulkanTexture> m_texturePool;
	GraphicPool<VulkanSampler> m_samplerPool;
	GraphicPool<VulkanBuffer> m_bufferPool;
	GraphicPool<VulkanShader> m_shaderPool;
	GraphicPool<VulkanProgram> m_programPool;
	GraphicPool<VulkanFramebuffer> m_framebufferPool;
	GraphicPool<VulkanPipeline> m_pipelinePool;
	GraphicPool<VulkanMaterial> m_materialPool;
	//GraphicPool<VulkanCommandList> m_commandPool;
};

};