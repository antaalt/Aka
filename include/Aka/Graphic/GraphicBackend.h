#pragma once

#include <Aka/Graphic/Texture2D.h>
#include <Aka/Graphic/Texture2DMultisample.h>
#include <Aka/Graphic/TextureCubeMap.h>
#include <Aka/Graphic/Device.h>
#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/ShaderMaterial.h>
#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/Mesh.h>
#include <Aka/Graphic/RenderPass.h>
#include <Aka/OS/FileSystem.h>

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

	friend class Texture2D;
	static Texture2D::Ptr createTexture2D(
		uint32_t width, uint32_t height, 
		TextureFormat format, TextureFlag flags, 
		const void* data
	);
	friend class Texture2DMultisample;
	static Texture2DMultisample::Ptr createTexture2DMultisampled(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		uint8_t samples,
		const void* data
	);
	friend class TextureCubeMap;
	static TextureCubeMap::Ptr createTextureCubeMap(
		uint32_t width, uint32_t height, 
		TextureFormat format, TextureFlag flags,
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
	static ShaderHandle compile(const char* content, ShaderType type);
	static void destroy(ShaderHandle handle);
	static Shader::Ptr createShader(ShaderHandle vert, ShaderHandle frag, const VertexAttribute* attributes, size_t count);
	static Shader::Ptr createShaderGeometry(ShaderHandle vert, ShaderHandle frag, ShaderHandle geometry, const VertexAttribute* attributes, size_t count);
	static Shader::Ptr createShaderCompute(ShaderHandle compute, const VertexAttribute* attributes, size_t count);

	friend class ShaderMaterial;
	static ShaderMaterial::Ptr createShaderMaterial(Shader::Ptr shader);
};

}