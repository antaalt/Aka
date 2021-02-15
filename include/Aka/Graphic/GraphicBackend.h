#pragma once

#include "Texture.h"
#include "Device.h"
#include "Shader.h"
#include "ShaderMaterial.h"
#include "Framebuffer.h"
#include "Mesh.h"
#include "RenderPass.h"
#include "../OS/FileSystem.h"

#if defined(AKA_USE_D3D11)
#include <d3d11.h>
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
	// Resize the graphic swapchain
	static void resize(uint32_t width, uint32_t height);
	// Get the size of the backbuffer
	static void getSize(uint32_t* width, uint32_t* height);
public:
	// Start a new frame
	static void frame();
	// Present the frame
	static void present();
	// Set the viewport
	static void viewport(int32_t x, int32_t y, uint32_t width, uint32_t height);
	// Get the backbuffer
	static Framebuffer::Ptr backbuffer();
	// Render a render pass
	static void render(RenderPass& renderPass);
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
	static Texture::Ptr createTexture(uint32_t width, uint32_t height, Texture::Format format, const uint8_t* data, Sampler sampler);

	friend class Framebuffer;
	static Framebuffer::Ptr createFramebuffer(uint32_t width, uint32_t height, Framebuffer::AttachmentType* attachment, size_t count, Sampler sampler);

	friend class Mesh;
	static Mesh::Ptr createMesh();

	friend class Shader;
	static ShaderID compile(const char* content, ShaderType type);
	static Shader::Ptr createShader(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Attributes>& attributes);

	friend class ShaderMaterial;
	static ShaderMaterial::Ptr createShaderMaterial(Shader::Ptr shader);
};

}