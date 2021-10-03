#if defined(AKA_USE_OPENGL)
#include "GLDevice.h"

#include "GLContext.h"
#include "GLBackbuffer.h"
#include "GLMesh.h"
#include "GLMaterial.h"
#include "GLBuffer.h"
#include "GLTexture.h"
#include "GLFramebuffer.h"
#include "GLProgram.h"

#include "Platform/GLFW3/PlatformGLFW3.h"

#include <Aka/OS/Logger.h>

namespace aka {

void APIENTRY openglCallbackFunction(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
) {
	std::string sourceType;
	switch (source) {
	case GL_DEBUG_SOURCE_API:
		sourceType = "GL";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		sourceType = "GL-window";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		sourceType = "GL-shader";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		sourceType = "GL-third-party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		sourceType = "GL-app";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		sourceType = "GL-other";
		break;
	default:
		return;
	}
	std::string errorType;
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		errorType = "error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		errorType = "deprecated-behaviour";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		errorType = "undefined-behaviour";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		errorType = "portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		errorType = "performance";
		break;
	case GL_DEBUG_TYPE_OTHER:
		errorType = "other";
		break;
	case GL_DEBUG_TYPE_MARKER:
		errorType = "marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		errorType = "push-group";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		errorType = "pop-group";
		break;
	default:
		return;
	}
	switch (severity) {
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		return; // Ignore notifications
	case GL_DEBUG_SEVERITY_LOW:
		aka::Logger::debug("[", sourceType, "][", errorType, "][low] ", message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		aka::Logger::warn("[", sourceType, "][", errorType, "][medium] ", message);
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		aka::Logger::error("[", sourceType, "][", errorType, "][high] ", message);
		break;
	default:
		return;
	}
}

GLDevice::GLDevice(uint32_t width, uint32_t height) :
	GraphicDevice(width, height)
{
	// Init glew for loading gl func
#if !defined(__APPLE__)
	glewExperimental = true; // Nécessaire dans le profil de base
	if (glewInit() != GLEW_OK) {
		Logger::error("Could not init GLEW");
		return;
	}
#endif

	// Setup log system
#if defined(AKA_DEBUG)
	if (glDebugMessageCallback) {
		Logger::debug("[GL] Setting up openGL callback.");
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(openglCallbackFunction, nullptr);
		GLuint unused = 0;
		glDebugMessageControl(
			GL_DONT_CARE,
			GL_DONT_CARE,
			GL_DONT_CARE,
			0,
			&unused,
			GL_TRUE
		);
	}
	else
		Logger::warn("[GL] glDebugMessageCallback not supported");
#endif

	// Check Features
	auto getUnsignedInteger = [](GLenum pname) -> uint32_t {
		GLint max = -1;
		glGetIntegerv(pname, &max);
		return (max == -1 ? 0 : max);
	};
	// We are using OpenGL 3.3 in this backend.
	m_features.api = GraphicApi::OpenGL;
	m_features.version.major = 3;
	m_features.version.minor = 3;
	m_features.profile = 330;

	m_features.maxTextureUnits = getUnsignedInteger(GL_MAX_TEXTURE_IMAGE_UNITS);
	m_features.maxTextureSize = getUnsignedInteger(GL_MAX_TEXTURE_SIZE);
	m_features.maxColorAttachments = getUnsignedInteger(GL_MAX_COLOR_ATTACHMENTS);
	m_features.maxElementIndices = getUnsignedInteger(GL_MAX_ELEMENTS_INDICES);
	m_features.maxElementVertices = getUnsignedInteger(GL_MAX_ELEMENTS_VERTICES);
	m_features.coordinates.clipSpacePositive = false; // GL clip space is [-1, 1]
	m_features.coordinates.originTextureBottomLeft = true; // GL start reading texture at bottom left.
	m_features.coordinates.originUVBottomLeft = true; // GL UV origin is bottom left
	m_features.coordinates.renderAxisYUp = true; // GL render axis y is up

	// Create backbuffer
	PlatformGLFW3* platform = reinterpret_cast<PlatformGLFW3*>(PlatformBackend::get());
	m_backbuffer = std::make_shared<GLBackbuffer>(platform->getGLFW3Handle(), width, height);
}

GLDevice::~GLDevice()
{
	m_backbuffer.reset();
}

void GLDevice::render(RenderPass& pass)
{
	{
		// Set framebuffer
		if (pass.framebuffer != m_backbuffer)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, ((GLFramebuffer*)pass.framebuffer.get())->m_framebufferID);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		// Clear
		if (pass.clear.mask != ClearMask::None)
		{
			GLenum glMask = 0;
			if ((pass.clear.mask & ClearMask::Color) == ClearMask::Color)
			{
				glClearColor(pass.clear.color.r, pass.clear.color.g, pass.clear.color.b, pass.clear.color.a);
				glMask |= GL_COLOR_BUFFER_BIT;
			}
			if ((pass.clear.mask & ClearMask::Depth) == ClearMask::Depth)
			{
				glDepthMask(true); // Ensure we can correctly clear the depth buffer.
				glClearDepth(pass.clear.depth);
				glMask |= GL_DEPTH_BUFFER_BIT;
			}
			if ((pass.clear.mask & ClearMask::Stencil) == ClearMask::Stencil)
			{
				glClearStencil(pass.clear.stencil);
				glMask |= GL_STENCIL_BUFFER_BIT;
			}
			glClear(glMask);
		}
	}

	{
		// Blending
		if (!pass.blend.enabled())
		{
			glDisable(GL_BLEND);
		}
		else
		{
			glEnable(GL_BLEND);
			GLenum colorModeSrc = glBlendMode(pass.blend.colorModeSrc);
			GLenum colorModeDst = glBlendMode(pass.blend.colorModeDst);
			GLenum alphaModeSrc = glBlendMode(pass.blend.alphaModeSrc);
			GLenum alphaModeDst = glBlendMode(pass.blend.alphaModeDst);
			GLenum colorOp = glBlendOp(pass.blend.colorOp);
			GLenum alphaOp = glBlendOp(pass.blend.alphaOp);
			glBlendEquationSeparate(colorOp, alphaOp);
			glBlendFuncSeparate(colorModeSrc, colorModeDst, alphaModeSrc, alphaModeDst);
			glBlendColor(
				pass.blend.blendColor.r,
				pass.blend.blendColor.g,
				pass.blend.blendColor.b,
				pass.blend.blendColor.a
			);
			glColorMask(
				(BlendMask)((int)pass.blend.mask & (int)BlendMask::Red) == BlendMask::Red,
				(BlendMask)((int)pass.blend.mask & (int)BlendMask::Green) == BlendMask::Green,
				(BlendMask)((int)pass.blend.mask & (int)BlendMask::Blue) == BlendMask::Blue,
				(BlendMask)((int)pass.blend.mask & (int)BlendMask::Alpha) == BlendMask::Alpha
			);
		}
	}
	{
		// Cull
		if (pass.cull.mode == CullMode::None)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
			switch (pass.cull.mode)
			{
			case CullMode::FrontFace:
				glCullFace(GL_FRONT);
				break;
			case CullMode::BackFace:
				glCullFace(GL_BACK);
				break;
			default:
			case CullMode::AllFace:
				glCullFace(GL_FRONT_AND_BACK);
				break;
			}
			switch (pass.cull.order)
			{
			case CullOrder::ClockWise:
				glFrontFace(GL_CW);
				break;
			default:
			case CullOrder::CounterClockWise:
				glFrontFace(GL_CCW);
				break;
			}
		}
	}

	{
		// Depth
		if (pass.depth.compare == DepthCompare::None)
		{
			glDisable(GL_DEPTH_TEST);
		}
		else
		{
			glEnable(GL_DEPTH_TEST);
			glDepthMask(pass.depth.mask);
			switch (pass.depth.compare)
			{
			default:
			case DepthCompare::Always:
				glDepthFunc(GL_ALWAYS);
				break;
			case DepthCompare::Never:
				glDepthFunc(GL_NEVER);
				break;
			case DepthCompare::Less:
				glDepthFunc(GL_LESS);
				break;
			case DepthCompare::Equal:
				glDepthFunc(GL_EQUAL);
				break;
			case DepthCompare::LessOrEqual:
				glDepthFunc(GL_LEQUAL);
				break;
			case DepthCompare::Greater:
				glDepthFunc(GL_GREATER);
				break;
			case DepthCompare::NotEqual:
				glDepthFunc(GL_NOTEQUAL);
				break;
			case DepthCompare::GreaterOrEqual:
				glDepthFunc(GL_GEQUAL);
				break;
			}
		}
	}

	{
		// Stencil
		if (!pass.stencil.enabled())
		{
			glDisable(GL_STENCIL_TEST);
		}
		else
		{
			glEnable(GL_STENCIL_TEST);
			glStencilMask(pass.stencil.writeMask);
			GLenum frontFunc = glStencilCompare(pass.stencil.front.mode);
			GLenum backFunc = glStencilCompare(pass.stencil.back.mode);
			glStencilFuncSeparate(GL_FRONT, frontFunc, 1, pass.stencil.readMask);
			glStencilFuncSeparate(GL_BACK, backFunc, 1, pass.stencil.readMask);
			glStencilOpSeparate(
				GL_FRONT,
				glStencilMode(pass.stencil.front.stencilFailed),
				glStencilMode(pass.stencil.front.stencilDepthFailed),
				glStencilMode(pass.stencil.front.stencilPassed)
			);
			glStencilOpSeparate(
				GL_BACK,
				glStencilMode(pass.stencil.back.stencilFailed),
				glStencilMode(pass.stencil.back.stencilDepthFailed),
				glStencilMode(pass.stencil.back.stencilPassed)
			);
		}
	}

	{
		// Viewport
		if (pass.viewport.w == 0 || pass.viewport.h == 0)
		{
			glViewport(
				0,
				0,
				static_cast<GLsizei>(pass.framebuffer->width()),
				static_cast<GLsizei>(pass.framebuffer->height())
			);
		}
		else
		{
			glViewport(
				static_cast<GLint>(pass.viewport.x),
				static_cast<GLint>(pass.viewport.y),
				static_cast<GLsizei>(pass.viewport.w),
				static_cast<GLsizei>(pass.viewport.h)
			);
		}
	}

	{
		// Scissor
		if (pass.scissor.w == 0 || pass.scissor.h == 0)
		{
			glDisable(GL_SCISSOR_TEST);
		}
		else
		{
			glEnable(GL_SCISSOR_TEST);
			glScissor(
				static_cast<GLint>(pass.scissor.x),
				static_cast<GLint>(pass.scissor.y),
				static_cast<GLsizei>(pass.scissor.w),
				static_cast<GLsizei>(pass.scissor.h)
			);
		}
	}

	{
		// Shader
		GLMaterial* material = (GLMaterial*)pass.material.get();
		material->use();
	}
	{
		// Mesh
		if (pass.submesh.mesh->isIndexed())
			pass.submesh.drawIndexed();
		else
			pass.submesh.draw();
	}
}

void GLDevice::dispatch(ComputePass& pass)
{
	// TODO assert GL version is 4.3 at least (minimum requirement for compute)
	{
		// Shader
		GLMaterial* material = (GLMaterial*)pass.material.get();
		material->use();
	}
	{
		glDispatchCompute(
			pass.groupCount.x,
			pass.groupCount.y,
			pass.groupCount.z
		);
		// TODO put barrier ?
	}
}

void GLDevice::copy(const Texture::Ptr& src, const Texture::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST) 
{
	AKA_ASSERT(src->format() == dst->format(), "Invalid format");
	AKA_ASSERT(regionSRC.x + regionSRC.width <= src->width() && regionSRC.y + regionSRC.height <= src->height(), "Region not in range.");
	AKA_ASSERT(regionDST.x + regionDST.width <= dst->width() && regionDST.y + regionDST.height <= dst->height(), "Region not in range.");
	AKA_ASSERT(regionSRC.width == regionDST.width && regionSRC.height == regionDST.height, "Region size invalid.");
	// TODO cache copyFBO to reuse it.
	GLuint copyFBO = 0;
	glGenFramebuffers(1, &copyFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, copyFBO);
	GLenum attachment = GL_COLOR_ATTACHMENT0;
	if (isDepthStencil(src->format()))
		attachment = GL_DEPTH_STENCIL_ATTACHMENT;
	else if (isDepth(src->format()))
		attachment = GL_DEPTH_ATTACHMENT;
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, (GLuint)src->handle().value(), regionSRC.level);
	glBindTexture(GL_TEXTURE_2D, (GLuint)dst->handle().value());
	glCopyTexSubImage2D(GL_TEXTURE_2D, regionDST.level, regionDST.x, regionDST.y, regionSRC.x, regionSRC.y, regionSRC.width, regionSRC.height);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &copyFBO);
	if ((TextureFlag::GenerateMips & dst->flags()) == TextureFlag::GenerateMips)
		dst->generateMips();
}

void GLDevice::blit(const Texture::Ptr& src, const Texture::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter) 
{
	AKA_ASSERT(regionSRC.x + regionSRC.width <= src->width() || regionSRC.y + regionSRC.height <= src->height(), "Region not in range");
	AKA_ASSERT(regionDST.x + regionDST.width <= dst->width() || regionDST.y + regionDST.height <= dst->height(), "Region not in range");
	GLuint blitFBO[2] = { 0 };
	GLenum attachment = GL_COLOR_ATTACHMENT0;
	GLenum mask = GL_COLOR_BUFFER_BIT;
	if (isDepthStencil(src->format()))
	{
		attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		mask = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
	}
	else if (isDepth(src->format()))
	{
		attachment = GL_DEPTH_ATTACHMENT;
		mask = GL_DEPTH_BUFFER_BIT;
	}
	glGenFramebuffers(2, blitFBO);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, blitFBO[0]); // src
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, attachment, GL_TEXTURE_2D, (GLuint)src->handle().value(), regionSRC.level);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blitFBO[1]); // dst
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D, (GLuint)dst->handle().value(), regionDST.level);
	glBlitFramebuffer(
		regionSRC.x, regionSRC.y, regionSRC.width, regionSRC.height,
		regionDST.x, regionDST.y, regionDST.width, regionDST.height,
		mask, glFilter(filter)
	);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(2, blitFBO);
	if ((TextureFlag::GenerateMips & dst->flags()) == TextureFlag::GenerateMips)
		dst->generateMips();
}

Device GLDevice::getDevice(uint32_t id)
{
	Device device;
	memcpy(device.vendor, glGetString(GL_VENDOR), 128);
	memcpy(device.renderer, glGetString(GL_RENDERER), 128);
	memcpy(device.version, glGetString(GL_VERSION), 128);
	device.memory = 0; // Can't get this info with GL
	// GL_SHADING_LANGUAGE_VERSION
	device.monitors;
	return device;
}

uint32_t GLDevice::getDeviceCount()
{
	return 0;
}

Texture2D::Ptr GLDevice::createTexture2D(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	const void* data
)
{
	return std::make_shared<GLTexture2D>(width, height, format, flags, data);
}

Texture2DMultisample::Ptr GLDevice::createTexture2DMultisampled(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	uint8_t samples,
	const void* data
)
{
	return std::make_shared<GLTexture2DMultisample>(width, height, format, flags, samples, data);
}

TextureCubeMap::Ptr GLDevice::createTextureCubeMap(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	const void* px, const void* nx,
	const void* py, const void* ny,
	const void* pz, const void* nz
)
{
	return std::make_shared<GLTextureCubeMap>(width, height, format, flags, px, nx, py, ny, pz, nz);
}

Framebuffer::Ptr GLDevice::createFramebuffer(Attachment* attachments, size_t count)
{
	return std::make_shared<GLFramebuffer>(attachments, count);
}

Buffer::Ptr GLDevice::createBuffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return std::make_shared<GLBuffer>(type, size, usage, access, data);
}

Mesh::Ptr GLDevice::createMesh()
{
	return std::make_shared<GLMesh>();
}

Shader::Ptr GLDevice::compile(const char* content, ShaderType type)
{
	return GLShader::compileGL(content, type);
}

Program::Ptr GLDevice::createVertexProgram(Shader::Ptr vert, Shader::Ptr frag, const VertexAttribute* attributes, size_t count)
{
	return std::make_shared<GLProgram>(vert, frag, nullptr, nullptr, attributes, count);
}
Program::Ptr GLDevice::createGeometryProgram(Shader::Ptr vert, Shader::Ptr frag, Shader::Ptr geometry, const VertexAttribute* attributes, size_t count)
{
	return std::make_shared<GLProgram>(vert, frag, geometry, nullptr, attributes, count);
}
Program::Ptr GLDevice::createComputeProgram(Shader::Ptr compute)
{
	VertexAttribute dummy{};
	return std::make_shared<GLProgram>(nullptr, nullptr, nullptr, compute, &dummy, 0);
}
Material::Ptr GLDevice::createMaterial(Program::Ptr shader)
{
	return std::make_shared<GLMaterial>(shader);
}

};
#endif