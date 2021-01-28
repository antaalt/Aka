#include "GraphicBackend.h"
#include "../../Core/Debug.h"
#include "../../Platform/Logger.h"

#include "Renderer/GLRenderer.h"
#include "Renderer/D3D11Renderer.h"

namespace aka {

GraphicBackend* GraphicBackend::m_backend = nullptr;

GraphicBackend::GraphicBackend(Window& window, uint32_t width, uint32_t height) :
	m_renderer(nullptr)
{
}

GraphicBackend::~GraphicBackend()
{
}


GraphicApi GraphicBackend::api()
{
	return m_backend->m_renderer->api();
}

void GraphicBackend::initialize(GraphicApi api, Window& window, uint32_t width, uint32_t height)
{
	if (m_backend != nullptr)
	{
		Logger::warn("Graphic backend already initialized.");
	}
	else
	{
		m_backend = new GraphicBackend(window, width, height);
		switch (api)
		{
#if defined(AKA_USE_D3D11)
		case GraphicApi::DirectX11:
			m_backend->m_renderer = new D3D11Renderer(window, width, height);
			break;
#endif
#if defined(AKA_USE_OPENGL)
		case GraphicApi::OpenGL:
			m_backend->m_renderer = new GLRenderer(window, width, height);
			break;
#endif
		default:
			Logger::critical("No renderer set");
			break;
		}
	}
}

void GraphicBackend::destroy()
{
	if (m_backend != nullptr)
	{
		delete m_backend->m_renderer;
		delete m_backend;
		m_backend = nullptr;
	}
}

void GraphicBackend::frame()
{
	ASSERT(m_backend != nullptr, "");
	m_backend->m_renderer->frame();
}

void GraphicBackend::present()
{
	ASSERT(m_backend != nullptr, "");
	m_backend->m_renderer->present();
}

void GraphicBackend::resize(uint32_t width, uint32_t height)
{
	ASSERT(m_backend != nullptr, "");
	m_backend->m_renderer->resize(width, height);
}

void GraphicBackend::viewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	m_backend->m_renderer->viewport(x, y, width, height);
}

Rect GraphicBackend::viewport()
{
	return m_backend->m_renderer->viewport();
}

Framebuffer::Ptr GraphicBackend::backbuffer()
{
	return m_backend->m_renderer->backbuffer();
}

void GraphicBackend::render(RenderPass& pass)
{
	m_backend->m_renderer->render(pass);
}

GraphicRenderer* GraphicBackend::renderer()
{
	return m_backend->m_renderer;
}

void GraphicBackend::screenshot(const Path& path)
{
	return m_backend->m_renderer->screenshot(path);
}

Device GraphicBackend::getDevice(uint32_t id)
{
	return m_backend->m_renderer->getDevice(id);
}

uint32_t GraphicBackend::deviceCount()
{
	return m_backend->m_renderer->deviceCount();
}

Texture::Ptr GraphicBackend::createTexture(uint32_t width, uint32_t height, Texture::Format format, const uint8_t* data, Sampler::Filter filter)
{
	return m_backend->m_renderer->createTexture(width, height, format, data, filter);
}

Framebuffer::Ptr GraphicBackend::createFramebuffer(uint32_t width, uint32_t height, Framebuffer::AttachmentType* attachment, size_t count, Sampler::Filter filter)
{
	return m_backend->m_renderer->createFramebuffer(width, height, attachment, count, filter);
}

Mesh::Ptr GraphicBackend::createMesh()
{
	return m_backend->m_renderer->createMesh();
}

ShaderID GraphicBackend::compile(const char* content, ShaderType type)
{
	return m_backend->m_renderer->compile(content, type);
}

Shader::Ptr GraphicBackend::createShader(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Attributes>& attributes)
{
	return m_backend->m_renderer->createShader(vert, frag, compute, attributes);
}

};