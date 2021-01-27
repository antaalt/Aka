#if defined(AKA_USE_OPENGL)
#pragma once

#include "../GraphicBackend.h"

namespace aka {

class GLRenderer : public GraphicRenderer
{
public:
	GLRenderer(Window& window, uint32_t width, uint32_t height);
	~GLRenderer();

	GraphicApi api() override;
	void resize(uint32_t width, uint32_t height) override;
	void frame() override;
	void present() override;
	void viewport(int32_t x, int32_t y, uint32_t width, uint32_t height) override;
	Rect viewport() override;
	Framebuffer::Ptr backbuffer() override;
	void render(RenderPass& renderPass) override;
protected:
	friend class GraphicBackend;
	friend struct Device;
	Device getDevice(uint32_t id) override;
	uint32_t deviceCount() override;

	friend class Texture;
	Texture::Ptr createTexture(uint32_t width, uint32_t height, Texture::Format format, const uint8_t* data, Sampler::Filter filter) override;

	friend class Framebuffer;
	Framebuffer::Ptr createFramebuffer(uint32_t width, uint32_t height, Framebuffer::AttachmentType* attachment, size_t count, Sampler::Filter filter) override;

	friend class Mesh;
	Mesh::Ptr createMesh() override;

	friend class Shader;
	ShaderID compile(const char* content, ShaderType type) override;
	Shader::Ptr createShader(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Attributes>& attributes) override;
protected:
	Framebuffer::Ptr m_backbuffer;
};

};

#endif