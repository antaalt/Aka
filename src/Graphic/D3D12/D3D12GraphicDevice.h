#pragma once

#if defined(AKA_USE_D3D12)

#include "D3D12Context.h"
#include "D3D12Texture.h"
#include "D3D12Buffer.h"
#include "D3D12Program.h"
#include "D3D12Pipeline.h"
#include "D3D12Framebuffer.h"
#include "D3D12CommandList.h"
#include "D3D12Swapchain.h"
#include "D3D12Sampler.h"

namespace aka {

class D3D12GraphicDevice : public GraphicDevice
{
public:
	D3D12GraphicDevice(PlatformDevice* device, const GraphicConfig& cfg);
	~D3D12GraphicDevice();

	GraphicAPI api() const override;

	// Device
	uint32_t getPhysicalDeviceCount() override;
	PhysicalDevice* getPhysicalDevice(uint32_t index) override;

	// Shaders
	Shader* compile(ShaderType type, const uint8_t* data, size_t size) override;
	void destroy(Shader* handle) override;

	// Programs
	Program* createProgram(Shader* vertex, Shader* fragment, Shader* geometry, const ShaderBindingState* bindings, uint32_t bindingCounts) override;
	void destroy(Program* handle) override;
	DescriptorSet* createDescriptorSet(const ShaderBindingState& bindings) override;
	void update(DescriptorSet* set) override;
	void destroy(DescriptorSet* set) override;

	// Textures
	Texture* createTexture(
		uint32_t width, uint32_t height, uint32_t depth,
		TextureType type,
		uint32_t levels, uint32_t layers,
		TextureFormat format,
		TextureFlag flags,
		const void* const* data
	) override;
	void upload(const Texture* texture, const void* const* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
	void download(const Texture* texture, void* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t mipLevel = 0, uint32_t layer = 0) override;
	void copy(const Texture* lhs, const Texture* rhs) override;
	void destroy(Texture* texture) override;

	// Sampler
	Sampler* createSampler(
		Filter filterMin,
		Filter filterMag,
		SamplerMipMapMode mipmapMode,
		uint32_t mipLevels,
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
		Program* program,
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

	D3D12Context& context() { return m_context; }
	D3D12Swapchain& swapchain() { return m_swapchain; }
private:
	D3D12Texture* makeTexture(
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

	D3D12Buffer* makeBuffer(
		BufferType type,
		uint32_t size,
		BufferUsage usage,
		BufferCPUAccess access,
		VkBuffer buffer,
		VkDeviceMemory memory
	);

	D3D12Shader* makeShader(
		ShaderType type,
		VkShaderModule module
	);
private:
	friend struct D3D12Swapchain;
	// Context
	D3D12Context m_context;

	// Frame
	D3D12Swapchain m_swapchain;

	// Pools
	GraphicPool<D3D12Texture> m_texturePool;
	GraphicPool<D3D12Sampler> m_samplerPool;
	GraphicPool<D3D12Buffer> m_bufferPool;
	GraphicPool<D3D12Shader> m_shaderPool;
	GraphicPool<D3D12Program> m_programPool;
	GraphicPool<D3D12Framebuffer> m_framebufferPool;
	GraphicPool<D3D12Pipeline> m_pipelinePool;
	GraphicPool<D3D12DescriptorSet> m_descriptorPool;
	//GraphicPool<D3D12CommandList> m_commandPool;
};

};
#endif