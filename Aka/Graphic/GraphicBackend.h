#pragma once

#include "Texture.h"
#include "Device.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "Mesh.h"
#include "RenderPass.h"

namespace aka {

class Window;

enum class GraphicApi {
#if defined(AKA_USE_OPENGL)
	OpenGL,
#endif
#if defined(AKA_USE_D3D11)
	DirectX11,
#endif
};

class GraphicRenderer
{
public:
	GraphicRenderer() {}
	virtual ~GraphicRenderer() {}

	virtual GraphicApi api() = 0;
	virtual void resize(uint32_t width, uint32_t height) = 0;
	virtual void frame() = 0;
	virtual void present() = 0;
	virtual void viewport(int32_t x, int32_t y, uint32_t width, uint32_t height) = 0;
	virtual Rect viewport() = 0;
	virtual Framebuffer::Ptr backbuffer() = 0;
	virtual void render(RenderPass& renderPass) = 0;
protected:
	friend class GraphicBackend;
	virtual Device getDevice(uint32_t id) = 0;
	virtual uint32_t deviceCount() = 0;
	virtual Texture::Ptr createTexture(uint32_t width, uint32_t height, Texture::Format format, const uint8_t* data, Sampler::Filter filter) = 0;
	virtual Framebuffer::Ptr createFramebuffer(uint32_t width, uint32_t height, Framebuffer::Attachment* attachment, size_t count) = 0;
	virtual Mesh::Ptr createMesh() = 0;
	virtual ShaderID compile(const char* content, ShaderType type) = 0;
	virtual Shader::Ptr createShader(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Attributes>& attributes) = 0;
};

class GraphicBackend {
	GraphicBackend(Window& window, uint32_t width, uint32_t height);
	GraphicBackend(GraphicBackend&) = delete;
	GraphicBackend& operator=(GraphicBackend&) = delete;
	~GraphicBackend();
	static GraphicBackend* m_backend;
public:
	// Get the current API
	static GraphicApi api();
	// Initialize the graphic API
	static void initialize(GraphicApi api, Window &window, uint32_t width, uint32_t height);
	// Destroy the graphic API
	static void destroy();
	// Resize the graphic swapchain
	static void resize(uint32_t width, uint32_t height);
public:
	// Start a new frame
	static void frame();
	// Present the frame
	static void present();
	// Set the viewport
	static void viewport(int32_t x, int32_t y, uint32_t width, uint32_t height);
	// Get the viewport
	static Rect viewport();
	// Get the backbuffer
	static Framebuffer::Ptr backbuffer();
	// Render a render pass
	static void render(RenderPass& renderPass);
	// Get the renderer
	static GraphicRenderer* renderer();
protected:
	friend struct Device;
	static Device getDevice(uint32_t id);
	static uint32_t deviceCount();

	friend class Texture;
	static Texture::Ptr createTexture(uint32_t width, uint32_t height, Texture::Format format, const uint8_t* data, Sampler::Filter filter);

	friend class Framebuffer;
	static Framebuffer::Ptr createFramebuffer(uint32_t width, uint32_t height, Framebuffer::Attachment* attachment, size_t count);

	friend class Mesh;
	static Mesh::Ptr createMesh();

	friend class Shader;
	static ShaderID compile(const char* content, ShaderType type);
	static Shader::Ptr createShader(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Attributes>& attributes);
protected:
	GraphicRenderer* m_renderer;
};

}