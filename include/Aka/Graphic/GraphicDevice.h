#pragma once

#include <type_traits>

#include <Aka/Core/Geometry.h>
#include <Aka/Core/Config.h>
#include <Aka/Core/StrictType.h>
#include <Aka/Platform/PlatformDevice.h>

#include <Aka/Graphic/Resource.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/Sampler.h>
#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/Pipeline.h>
#include <Aka/Graphic/CommandList.h>
#include <Aka/Graphic/DescriptorSet.h>
#include <Aka/Graphic/PhysicalDevice.h>
#include <Aka/Graphic/Fence.h>

namespace aka {
namespace gfx {

enum class GraphicAPI : uint8_t
{
	Unknown,

	Vulkan,
	DirectX12,

	First = Vulkan,
	Last = DirectX12,
};

struct GraphicConfig
{
	GraphicAPI api;
};

CREATE_STRICT_TYPE(uint32_t, FrameIndex)
CREATE_STRICT_TYPE(uint32_t, ImageIndex)

static constexpr uint32_t MaxFrameInFlight = 1; // number of frames to deal with concurrently

struct Frame // TODO FrameHandle instead.
{
protected:
	ImageIndex m_image;
};

struct Backbuffer : Resource
{ 
	Backbuffer(const char* name) : Resource(name, ResourceType::Framebuffer) {}
	std::vector<FramebufferHandle> handles;
};

using BackbufferHandle = ResourceHandle<Backbuffer>;


enum class SwapchainStatus
{
	Unknown,

	Ok,
	Recreated,
	Error,

	First = Ok,
	Last = Error,
};


class AKA_NO_VTABLE GraphicDevice
{
public:
	virtual ~GraphicDevice() {}

	static GraphicDevice* create(PlatformDevice* device, const GraphicConfig& cfg);
	static void destroy(GraphicDevice* device);

	virtual GraphicAPI api() const = 0;
	// Set resource name
	virtual void name(const Resource* resource, const char* name) = 0;

	// Shaders
	virtual ShaderHandle createShader(ShaderType type, const void* data, size_t size) = 0;
	virtual void destroy(ShaderHandle handle) = 0;
	virtual const Shader* get(ShaderHandle handle) = 0;

	// Programs
	virtual ProgramHandle createGraphicProgram(const char* name, ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, const ShaderBindingState* bindings, uint32_t bindingCounts) = 0;
	virtual ProgramHandle createComputeProgram(const char* name, ShaderHandle compute, const ShaderBindingState* bindings, uint32_t bindingCounts) = 0;
	virtual void destroy(ProgramHandle program) = 0;
	virtual const Program* get(ProgramHandle program) = 0;

	// Descriptor sets
	virtual DescriptorSetHandle createDescriptorSet(const char* name, const ShaderBindingState& bindings) = 0;
	virtual void update(DescriptorSetHandle set, const DescriptorSetData& data) = 0;
	virtual void destroy(DescriptorSetHandle set) = 0;
	virtual const DescriptorSet* get(DescriptorSetHandle set) = 0;

	// Device
	virtual uint32_t getPhysicalDeviceCount() = 0;
	virtual const PhysicalDevice* getPhysicalDevice(uint32_t index) = 0;

	// Framebuffer
	virtual FramebufferHandle createFramebuffer(const char* name, RenderPassHandle handle, const Attachment* attachments, uint32_t count, const Attachment* depth) = 0;
	virtual void destroy(FramebufferHandle framebuffer) = 0;
	virtual void destroy(BackbufferHandle backbuffer) = 0;
	virtual BackbufferHandle createBackbuffer(RenderPassHandle handle) = 0;
	virtual RenderPassHandle createBackbufferRenderPass(AttachmentLoadOp loadOp = AttachmentLoadOp::Clear, AttachmentStoreOp storeOp = AttachmentStoreOp::Store, ResourceAccessType initialLayout = ResourceAccessType::Undefined, ResourceAccessType finalLayout = ResourceAccessType::Present) = 0;
	virtual FramebufferHandle get(BackbufferHandle handle, Frame* frame) = 0;
	virtual const Framebuffer* get(FramebufferHandle handle) = 0;

	// RenderPass
	virtual RenderPassHandle createRenderPass(const char* name, const RenderPassState& state) = 0;
	virtual void destroy(RenderPassHandle framebuffer) = 0;
	virtual const RenderPass* get(RenderPassHandle handle) = 0;

	// Buffers
	virtual BufferHandle createBuffer(const char* name, BufferType type, uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data = nullptr) = 0;
	virtual void upload(BufferHandle buffer, const void* data, uint32_t offset, uint32_t size) = 0;
	virtual void download(BufferHandle buffer, void* data, uint32_t offset, uint32_t size) = 0;
	virtual void* map(BufferHandle buffer, BufferMap map) = 0;
	virtual void unmap(BufferHandle buffer) = 0;
	virtual void destroy(BufferHandle buffer) = 0;
	virtual const Buffer* get(BufferHandle handle) = 0;

	// Textures
	virtual TextureHandle createTexture(const char* name, uint32_t width, uint32_t height, uint32_t depth, TextureType type, uint32_t levels, uint32_t layers, TextureFormat format, TextureUsage flags, const void* const* data = nullptr) = 0;
	virtual void upload(TextureHandle texture, const void* const* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
	virtual void download(TextureHandle texture, void* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t mipLevel = 0, uint32_t layer = 0) = 0;
	virtual void copy(TextureHandle lhs, TextureHandle rhs) = 0;
	virtual void destroy(TextureHandle texture) = 0;
	virtual void transition(TextureHandle texture, ResourceAccessType src, ResourceAccessType dst) = 0;
	virtual const Texture* get(TextureHandle handle) = 0;

	// Samplers
	virtual SamplerHandle createSampler(const char* name, Filter filterMin, Filter filterMag, SamplerMipMapMode mipmapMode, SamplerAddressMode wrapU, SamplerAddressMode wrapV, SamplerAddressMode wrapW, float anisotropy) = 0;
	virtual void destroy(SamplerHandle sampler) = 0;
	virtual const Sampler* get(SamplerHandle handle) = 0;

	// Pipelines
	virtual GraphicPipelineHandle createGraphicPipeline(
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
	) = 0;
	virtual ComputePipelineHandle createComputePipeline(
		ProgramHandle program,
		uint32_t groupCountX,
		uint32_t groupCountY,
		uint32_t groupCountZ
	) = 0;
	virtual void destroy(GraphicPipelineHandle handle) = 0;
	virtual void destroy(ComputePipelineHandle handle) = 0;
	virtual const GraphicPipeline* get(GraphicPipelineHandle handle) = 0;
	virtual const ComputePipeline* get(ComputePipelineHandle handle) = 0;

	// Fence
	virtual FenceHandle createFence(const char* name) = 0;
	virtual void destroy(FenceHandle handle) = 0;
	virtual const Fence* get(FenceHandle handle) = 0;
	virtual void wait(FenceHandle handle, FenceValue waitValue) = 0;
	virtual void signal(FenceHandle handle, FenceValue value) = 0;
	virtual FenceValue read(FenceHandle handle) = 0;

	// Command
	virtual CommandList* acquireCommandList(QueueType queue) = 0;
	virtual void release(CommandList* cmd) = 0;
	// Frame command lists
	virtual CommandList* getCopyCommandList(Frame* frame) = 0;
	virtual CommandList* getGraphicCommandList(Frame* frame) = 0;
	virtual CommandList* getComputeCommandList(Frame* frame) = 0;
	// Command submit
	virtual void submit(CommandList* command, FenceHandle handle = FenceHandle::null, FenceValue waitValue = 0U, FenceValue signalValue = 0U) = 0;
	virtual void wait(QueueType queue) = 0;

	// Screenshot of the backbuffer
	virtual void screenshot(void* data) = 0;

	// Frame
	virtual Frame* frame() = 0;
	virtual SwapchainStatus present(Frame* frame) = 0;
	virtual void wait() = 0;
};

};
};