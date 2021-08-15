#pragma once

#include "Texture.h"
#include "Device.h"
#include "Shader.h"
#include "ShaderMaterial.h"
#include "Framebuffer.h"
#include "Buffer.h"
#include "Mesh.h"
#include "RenderPass.h"
#include "../OS/FileSystem.h"

#if defined(AKA_USE_D3D11)
struct ID3D11Device;
struct ID3D11DeviceContext;
#endif

namespace aka {

class Window;

enum class GraphicApi {
	OpenGL,
	DirectX11,
};

class GraphicBackend {
	GraphicBackend() {}
	GraphicBackend(GraphicBackend&) = delete;
	GraphicBackend& operator=(GraphicBackend&) = delete;
	~GraphicBackend() {}
public:
	// Get the current API
	static GraphicApi api();
	// Initialize the graphic API
	static void initialize(uint32_t width, uint32_t height);
	// Destroy the graphic API
	static void destroy();
public:
	// Start a new frame
	static void frame();
	// Present the frame
	static void present();
	// Get the backbuffer
	static Framebuffer::Ptr backbuffer();
	// Render a render pass
	static void render(RenderPass& renderPass);
	// Dispatch a compute pass
	static void dispatch(ComputePass& computePass);
	// Take a screenshot
	static void screenshot(const Path& path);
	// Set the vsync
	static void vsync(bool enabled);
public:
#if defined(AKA_USE_D3D11)
	// Get D3D11 device
	static ID3D11Device* getD3D11Device();
	// Get D3D11 device context
	static ID3D11DeviceContext* getD3D11DeviceContext();
#endif
protected:
	friend struct Device;
	static Device getDevice(uint32_t id);
	static uint32_t deviceCount();

	friend class Texture;
	static Texture::Ptr createTexture2D(
		uint32_t width, uint32_t height, 
		TextureFormat format, TextureComponent component, TextureFlag flags, 
		Sampler sampler,
		const void* data
	);
	static Texture::Ptr createTexture2DMultisampled(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureComponent component, TextureFlag flags,
		Sampler sampler,
		const void* data,
		uint8_t samples
	);
	static Texture::Ptr createTextureCubeMap(
		uint32_t width, uint32_t height, 
		TextureFormat format, TextureComponent component, TextureFlag flags,
		Sampler sampler,
		const void* px, const void* nx,
		const void* py, const void* ny,
		const void* pz, const void* nz
	);

	friend class Framebuffer;
	static Framebuffer::Ptr createFramebuffer(FramebufferAttachment* attachments, size_t count);

	friend class Buffer;
	static Buffer::Ptr createBuffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data);

	friend class Mesh;
	static Mesh::Ptr createMesh();

	friend class Shader;
	static ShaderID compile(const char* content, ShaderType type);
	static Shader::Ptr createShader(ShaderID vert, ShaderID frag, const std::vector<Attributes>& attributes);
	static Shader::Ptr createShaderGeometry(ShaderID vert, ShaderID frag, ShaderID geometry, const std::vector<Attributes>& attributes);
	static Shader::Ptr createShaderCompute(ShaderID compute, const std::vector<Attributes>& attributes);

	friend class ShaderMaterial;
	static ShaderMaterial::Ptr createShaderMaterial(Shader::Ptr shader);
};

}