#if defined(AKA_USE_DUMMY)

#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/Drawing/Renderer2D.h>
#include <Aka/Core/Debug.h>
#include <Aka/Core/Event.h>
#include <Aka/OS/Logger.h>
#include <Aka/Platform/Platform.h>
#include <Aka/Platform/PlatformDevice.h>

namespace aka {

class DummyTexture2D : public Texture2D
{
public:
	DummyTexture2D(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		const void* data
	) : Texture2D(width, height, format, flags) {}
	~DummyTexture2D() {}
	void upload(const void* data, uint32_t level) override {}
	void upload(const Rect& rect, const void* data, uint32_t level) override {}
	void download(void* data, uint32_t level) override {}
	void copy(const Texture2D::Ptr& dst, const Rect& rectSRC, const Rect& rectDST, uint32_t level) override {}
	void blit(const Texture2D::Ptr& dst, const Rect& rectSRC, const Rect& rectDST, TextureFilter filter, uint32_t level) override {}
	TextureHandle handle() const override { return TextureHandle(0); }
	void generateMips() override {}
};

class DummyTexture2DMultisample : public Texture2DMultisample
{
public:
	DummyTexture2DMultisample(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		uint8_t samples,
		const void* data
	) : Texture2DMultisample(width, height, format, flags) {}
	~DummyTexture2DMultisample() {}
	void upload(const void* data) override {}
	void upload(const Rect& rect, const void* data) override {}
	void download(void* data) override {}
	TextureHandle handle() const override { return TextureHandle(0); }
	void generateMips() override {}
};


class DummyTextureCubeMap : public TextureCubeMap
{
public:
	DummyTextureCubeMap(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		const void* px, const void* nx,
		const void* py, const void* ny,
		const void* pz, const void* nz
	) : TextureCubeMap(width, height, format, flags) {}
	~DummyTextureCubeMap() {}
	void upload(const void* data, uint32_t layer, uint32_t level) override {}
	void upload(const Rect& rect, const void* data, uint32_t layer, uint32_t level) override {}
	void download(void* data, uint32_t layer, uint32_t level) override {}
	TextureHandle handle() const override { return TextureHandle(0); }
	void generateMips() override {}
};

class DummyFramebuffer : public Framebuffer
{
public:
	DummyFramebuffer(Attachment* attachments, size_t count) : Framebuffer(attachments, count) {}
	~DummyFramebuffer() {}
	void clear(const color4f& color, float depth, int stencil, ClearMask mask) override {}
	void blit(Framebuffer::Ptr src, Rect rectSrc, Rect rectDst, AttachmentType type, TextureFilter filter) override {}
	void set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag, uint32_t layer, uint32_t level) override
	{
		// Check attachment
		Attachment newAttachment = Attachment{ type, texture, flag, layer, level };
		if (!valid(newAttachment))
		{
			Logger::error("Incompatible attachment set for framebuffer");
			return;
		}
		Attachment* attachment = getAttachment(type);
		if (attachment == nullptr)
		{
			m_attachments.push_back(newAttachment);
			attachment = &m_attachments.back();
		}
		else
		{
			if (attachment->texture == texture && attachment->flag == flag && attachment->layer == layer && attachment->level == level)
				return; // Everything already set.
			attachment->texture = texture;
			attachment->flag = flag;
			attachment->layer = layer;
			attachment->level = level;
		}
		// TODO Recompute size
		//computeSize();
	}
};

class DummyBackBuffer : public Framebuffer, EventListener<BackbufferResizeEvent>
{
public:
	DummyBackBuffer(uint32_t width, uint32_t height) : Framebuffer(width, height) {}
	~DummyBackBuffer() {}

	void resize(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;
	}
	void clear(const color4f& color, float depth, int stencil, ClearMask mask) override {}
	void blit(Framebuffer::Ptr src, Rect rectSrc, Rect rectDst, AttachmentType type, TextureFilter filter) override {}
	void set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag, uint32_t layer, uint32_t level) override {}
	void onReceive(const BackbufferResizeEvent& event) override
	{
		if (event.width != 0 && event.height != 0)
			resize(event.width, event.height);
	}
};

class DummyBuffer : public Buffer
{
public:
	DummyBuffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data) :
		Buffer(type, size, usage, access),
		m_mapped(nullptr)
	{
		reallocate(size, data);
	}
	~DummyBuffer() {
		if (m_mapped)
			delete[] m_mapped;
	}

	void reallocate(size_t size, const void* data = nullptr) override {}
	void upload(const void* data, size_t size, size_t offset = 0) override{}
	void upload(const void* data) override{}
	void download(void* data, size_t size, size_t offset = 0) override{}
	void download(void* data) override{};
	void* map(BufferMap bufferMap) override { if (!m_mapped) m_mapped = new char[m_size]; return m_mapped; }
	void unmap()  override { if (m_mapped) { delete[] m_mapped; m_mapped = nullptr; } }
	BufferHandle handle() const  override{ return BufferHandle(0); }
private:
	void* m_mapped = nullptr;
};

class DummyMesh : public Mesh
{
public:
	DummyMesh() : Mesh(){}
	~DummyMesh() {}
	void upload(const VertexAccessor* vertexAccessor, size_t count, const IndexAccessor& indexAccessor) override{}
	void upload(const VertexAccessor* vertexAccessor, size_t count) override{}
	void draw(PrimitiveType type, uint32_t vertexCount, uint32_t vertexOffset) const override{}
	void drawIndexed(PrimitiveType type, uint32_t indexCount, uint32_t indexOffset) const override{}
};

class DummyShader : public Shader
{
public:
	DummyShader(){}
	~DummyShader() {}
};

class DummyProgram : public Program
{
public:
	DummyProgram(Shader::Ptr vert, Shader::Ptr frag, Shader::Ptr geometry, Shader::Ptr compute, const VertexAttribute* attributes, size_t count) :
		Program(attributes, count) {}
	~DummyProgram() {}
};

class DummyMaterial : public Material
{
public:
	DummyMaterial(Program::Ptr program) :
		Material(program)
	{
		uint32_t textureCount = 0;
		uint32_t bufferCount = 0;
		for (const Uniform& uniform : *m_program)
		{
			// Create textures & data buffers
			switch (uniform.type)
			{
			case UniformType::Buffer:
				bufferCount = max(bufferCount, uniform.binding + uniform.count);
				break;
			case UniformType::Texture2D:
			case UniformType::TextureCubemap:
			case UniformType::Texture2DMultisample:
				textureCount = max(textureCount, uniform.binding + uniform.count);
				break;
			case UniformType::Int:
			case UniformType::UnsignedInt:
			case UniformType::Float:
			case UniformType::Vec2:
			case UniformType::Vec3:
			case UniformType::Vec4:
			case UniformType::Mat3:
			case UniformType::Mat4:
			default:
				break;
			}
		}
		m_textures.resize(textureCount, nullptr);
		m_samplers.resize(textureCount, TextureSampler::nearest);
		m_buffers.resize(bufferCount, nullptr);
	}
	~DummyMaterial()
	{
	}
};

struct DummyContext {
	std::shared_ptr<DummyBackBuffer> backbuffer = nullptr;
	GraphicFeatures features;
};

static DummyContext ctx{};

void GraphicBackend::initialize(uint32_t width, uint32_t height)
{
	ctx.backbuffer = std::make_shared<DummyBackBuffer>(width, height);
	ctx.features.api = GraphicApi::None;
	ctx.features.version.major = 0;
	ctx.features.version.minor = 0;
	ctx.features.profile = 0;
	ctx.features.maxTextureUnits = 0;
	ctx.features.maxTextureSize = 0;
	ctx.features.maxColorAttachments = 0;
	ctx.features.maxElementIndices = 0;
	ctx.features.maxElementVertices = 0;
	ctx.features.conventions.clipSpacePositive = true;
	ctx.features.conventions.originTextureBottomLeft = true;
	ctx.features.conventions.originUVBottomLeft = true;
	ctx.features.conventions.renderAxisYUp = true;
}

void GraphicBackend::destroy()
{
}

GraphicApi GraphicBackend::api()
{
	return ctx.features.api;
}

void GraphicBackend::frame()
{
}

void GraphicBackend::present()
{
}

Framebuffer::Ptr GraphicBackend::backbuffer()
{
	return ctx.backbuffer;
}

void GraphicBackend::render(RenderPass& pass)
{
}
void GraphicBackend::dispatch(ComputePass& computePass)
{
}
void GraphicBackend::screenshot(const Path& path)
{
}

void GraphicBackend::vsync(bool enabled)
{
}

Device GraphicBackend::getDevice(uint32_t id)
{
	return Device{};
}

uint32_t Application::graphicCount()
{
	return 0;
}

Texture2D::Ptr GraphicBackend::createTexture2D(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* data)
{
	return std::make_shared<DummyTexture2D>(width, height, format, flags, data);
}

TextureCubeMap::Ptr GraphicBackend::createTextureCubeMap(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	const void* px, const void* nx,
	const void* py, const void* ny,
	const void* pz, const void* nz
)
{
	return std::make_shared<DummyTextureCubeMap>(width, height, format, flags, px, nx, py, ny, pz, nz);
}

Texture2DMultisample::Ptr GraphicBackend::createTexture2DMultisampled(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	uint8_t samples,
	const void* data
)
{
	return std::make_shared<DummyTexture2DMultisample>(width, height, format, flags, samples, data);
}

Framebuffer::Ptr GraphicBackend::createFramebuffer(Attachment* attachments, size_t count)
{
	return std::make_shared<DummyFramebuffer>(attachments, count);
}

Buffer::Ptr GraphicBackend::createBuffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return std::make_shared<DummyBuffer>(type, size, usage, access, data);
}

Mesh::Ptr GraphicBackend::createMesh()
{
	return std::make_shared<DummyMesh>();
}

Shader::Ptr GraphicBackend::compile(const char* content, ShaderType type)
{
	return std::make_shared<DummyShader>();
}

Program::Ptr GraphicBackend::createVertexProgram(Shader::Ptr vert, Shader::Ptr frag, const VertexAttribute* attributes, size_t count)
{
	return std::make_shared<DummyProgram>(vert, frag, nullptr, nullptr, attributes, count);
}

Program::Ptr GraphicBackend::createComputeProgram(Shader::Ptr compute)
{
	return std::make_shared<DummyProgram>(nullptr, nullptr, nullptr, compute, nullptr, 0);
}

Program::Ptr GraphicBackend::createGeometryProgram(Shader::Ptr vert, Shader::Ptr frag, Shader::Ptr geometry, const VertexAttribute* attributes, size_t count)
{
	return std::make_shared<DummyProgram>(vert, frag, geometry, nullptr, attributes, count);
}

Material::Ptr GraphicBackend::createMaterial(Program::Ptr program)
{
	return std::make_shared<DummyMaterial>(program);
}

};
#endif