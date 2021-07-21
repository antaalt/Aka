#if defined(AKA_USE_OPENGL)
#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/Core/Debug.h>
#include <Aka/Core/Event.h>
#include <Aka/OS/Logger.h>
#include <Aka/OS/Image.h>
#include <Aka/Platform/PlatformBackend.h>

#if defined(AKA_PLATFORM_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
#define GLEW_NO_GLU
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cstring>

#define GL_CHECK_RESULT(result)                \
{                                              \
	GLenum res = (result);                     \
	if (GL_NO_ERROR != res) {                  \
		char buffer[256];                      \
		snprintf(                              \
			buffer,                            \
			256,                               \
			"%s (%s at %s:%d)",                \
			glewGetErrorString(res),           \
			AKA_STRINGIFY(result),             \
			__FILE__,                          \
			__LINE__                           \
		);                                     \
		std::cerr << buffer << std::endl;      \
		throw std::runtime_error(res, buffer); \
	}                                          \
}

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

std::string glGetErrorString(GLenum error)
{
	switch (error)
	{
	case GL_INVALID_ENUM:
		return "INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "INVALID_OPERATION";
	case GL_STACK_OVERFLOW:
		return "STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:
		return "STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:
		return "OUT_OF_MEMORY";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "INVALID_FRAMEBUFFER_OPERATION";
	default:
		return "ERROR_UNKNOWN";
	}
}

namespace aka {

namespace gl {
GLenum attachmentType(FramebufferAttachmentType type)
{
	switch (type)
	{
	default:
		Logger::warn("[GL] Framebuffer attachment type not defined : ", (int)type);
		return GL_COLOR_ATTACHMENT0;
	case FramebufferAttachmentType::Color0:
		return GL_COLOR_ATTACHMENT0;
	case FramebufferAttachmentType::Color1:
		return GL_COLOR_ATTACHMENT1;
	case FramebufferAttachmentType::Color2:
		return GL_COLOR_ATTACHMENT2;
	case FramebufferAttachmentType::Color3:
		return GL_COLOR_ATTACHMENT3;
	case FramebufferAttachmentType::Depth:
		return GL_DEPTH_ATTACHMENT;
	case FramebufferAttachmentType::Stencil:
		return GL_STENCIL_ATTACHMENT;
	case FramebufferAttachmentType::DepthStencil:
		return GL_DEPTH_STENCIL_ATTACHMENT;
	}
}

GLenum filter(Sampler::Filter type, Sampler::MipMapMode mode = Sampler::MipMapMode::None)
{
	switch (mode)
	{
	case Sampler::MipMapMode::Linear:
		if (type == Sampler::Filter::Linear)
			return GL_LINEAR_MIPMAP_LINEAR; // trilinear
		else if (type == Sampler::Filter::Nearest)
			return GL_LINEAR_MIPMAP_NEAREST;
		break;
	case Sampler::MipMapMode::Nearest:
		if (type == Sampler::Filter::Linear)
			return GL_NEAREST_MIPMAP_LINEAR; // bilinear
		else if (type == Sampler::Filter::Nearest)
			return GL_NEAREST_MIPMAP_NEAREST;
		break;
	default:
	case Sampler::MipMapMode::None:
		if (type == Sampler::Filter::Linear)
			return GL_LINEAR;
		else if (type == Sampler::Filter::Nearest)
			return GL_NEAREST;
		break;
	}
	Logger::warn("[GL] Invalid filtering values : ", (int)type, " & ", (int)mode);
	return 0;
}

GLenum wrap(Sampler::Wrap wrap) {
	switch (wrap) {
	default:
	case Sampler::Wrap::Repeat:
		return GL_REPEAT;
	case Sampler::Wrap::Mirror:
		return GL_MIRRORED_REPEAT;
	case Sampler::Wrap::ClampToEdge:
		return GL_CLAMP_TO_EDGE;
	case Sampler::Wrap::ClampToBorder:
		return GL_CLAMP_TO_BORDER;
	}
}

GLenum componentInternal(TextureComponent component) {
	switch (component) {
	default:
		throw std::runtime_error("Not implemneted");
	case TextureComponent::R:
		return GL_RED;
	case TextureComponent::R8:
		return GL_R8;
	case TextureComponent::R16:
		return GL_R16;
	case TextureComponent::R16F:
		return GL_R16F;
	case TextureComponent::R32F:
		return GL_R32F;
	case TextureComponent::RG:
		return GL_RG;
	case TextureComponent::RG8:
		return GL_RG8;
	case TextureComponent::RG16:
		return GL_RG16;
	case TextureComponent::RGB:
		return GL_RGB;
	case TextureComponent::RGB8:
		return GL_RGB8;
	case TextureComponent::RGB16:
		return GL_RGB16;
	case TextureComponent::RGB16F:
		return GL_RGB16F;
	case TextureComponent::RGB32F:
		return GL_RGB32F;
	case TextureComponent::RGBA:
		return GL_RGBA;
	case TextureComponent::RGBA8:
		return GL_RGBA8;
	case TextureComponent::RGBA16F:
		return GL_RGBA16F;
	case TextureComponent::RGBA32F:
		return GL_RGBA32F;
	case TextureComponent::Depth:
		return GL_DEPTH_COMPONENT;
	case TextureComponent::Depth16:
		return GL_DEPTH_COMPONENT16;
	case TextureComponent::Depth32:
		return GL_DEPTH_COMPONENT32;
	case TextureComponent::Depth32F:
		return GL_DEPTH_COMPONENT32F;
	case TextureComponent::DepthStencil:
		return GL_DEPTH_STENCIL;
	case TextureComponent::Depth24Stencil8:
		return GL_DEPTH24_STENCIL8;
	case TextureComponent::Depth32FStencil8:
		return GL_DEPTH32F_STENCIL8;
	}
}

GLenum component(TextureComponent component) {
	switch (component) {
	default:
		throw std::runtime_error("Not implemneted");
	case TextureComponent::R:
	case TextureComponent::R8:
	case TextureComponent::R16:
	case TextureComponent::R16F:
	case TextureComponent::R32F:
		return GL_RED;
	case TextureComponent::RG:
	case TextureComponent::RG8:
	case TextureComponent::RG16:
		return GL_RG;
	case TextureComponent::RGB:
	case TextureComponent::RGB8:
	case TextureComponent::RGB16:
	case TextureComponent::RGB16F:
	case TextureComponent::RGB32F:
		return GL_RGB;
	case TextureComponent::RGBA:
	case TextureComponent::RGBA8:
	case TextureComponent::RGBA16F:
	case TextureComponent::RGBA32F:
		return GL_RGBA;
	case TextureComponent::Depth:
	case TextureComponent::Depth16:
	case TextureComponent::Depth32:
	case TextureComponent::Depth32F:
		return GL_DEPTH_COMPONENT;
	case TextureComponent::Depth24Stencil8:
	case TextureComponent::Depth32FStencil8:
	case TextureComponent::DepthStencil:
		return GL_DEPTH_STENCIL;
	}
}

GLenum type(TextureType type) {
	switch (type) {
	default:
		throw std::runtime_error("Not implemneted");
	case TextureType::Texture2D:
		return GL_TEXTURE_2D;
	case TextureType::TextureCubemap:
		return GL_TEXTURE_CUBE_MAP;
	case TextureType::Texture2DMultisample:
		return GL_TEXTURE_2D_MULTISAMPLE;
	}
}

GLenum format(TextureFormat format) {
	switch (format) {
	default:
		throw std::runtime_error("Not implemneted");
	case TextureFormat::Byte:
		return GL_BYTE;
	case TextureFormat::UnsignedByte:
		return GL_UNSIGNED_BYTE;
	case TextureFormat::Short:
		return GL_SHORT;
	case TextureFormat::UnsignedShort:
		return GL_UNSIGNED_SHORT;
	case TextureFormat::Int:
		return GL_INT;
	case TextureFormat::UnsignedInt:
		return GL_UNSIGNED_INT;
	case TextureFormat::Half:
		return GL_HALF_FLOAT;
	case TextureFormat::Float:
		return GL_FLOAT;
	case TextureFormat::UnsignedInt248:
		return GL_UNSIGNED_INT_24_8;
	case TextureFormat::Float32UnsignedInt248:
		return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
	}
}

GLenum format(IndexFormat format) {
	switch (format) {
	default:
		throw std::runtime_error("Not implemneted");
	case IndexFormat::UnsignedByte:
		return GL_UNSIGNED_BYTE;
	case IndexFormat::UnsignedShort:
		return GL_UNSIGNED_SHORT;
	case IndexFormat::UnsignedInt:
		return GL_UNSIGNED_INT;
	}
}

GLenum format(VertexFormat format) {
	switch (format) {
	default:
		throw std::runtime_error("Not implemneted");
	case VertexFormat::Float:
		return GL_FLOAT;
	case VertexFormat::Double:
		return GL_DOUBLE;
	case VertexFormat::Byte:
		return GL_BYTE;
	case VertexFormat::UnsignedByte:
		return GL_UNSIGNED_BYTE;
	case VertexFormat::Short:
		return GL_SHORT;
	case VertexFormat::UnsignedShort:
		return GL_UNSIGNED_SHORT;
	case VertexFormat::Int:
		return GL_INT;
	case VertexFormat::UnsignedInt:
		return GL_UNSIGNED_INT;
	}
}

GLuint getType(ShaderType type)
{
	switch (type)
	{
	case ShaderType::Vertex:
		return GL_VERTEX_SHADER;
	case ShaderType::TesselationControl:
		return GL_TESS_CONTROL_SHADER;
	case ShaderType::TesselationEvaluation:
		return GL_TESS_EVALUATION_SHADER;
	case ShaderType::Fragment:
		return GL_FRAGMENT_SHADER;
	case ShaderType::Geometry:
		return GL_GEOMETRY_SHADER;
	case ShaderType::Compute:
		return GL_COMPUTE_SHADER;
	default:
		Logger::error("[GL] Incorrect shader type");
		return 0;
	}
}

GLenum blendMode(BlendMode mode)
{
	switch (mode)
	{
	case BlendMode::Zero:
		return GL_ZERO;
	default:
	case BlendMode::One:
		return GL_ONE;
	case BlendMode::SrcColor:
		return GL_SRC_COLOR;
	case BlendMode::OneMinusSrcColor:
		return GL_ONE_MINUS_SRC_COLOR;
	case BlendMode::DstColor:
		return GL_DST_COLOR;
	case BlendMode::OneMinusDstColor:
		return GL_ONE_MINUS_DST_COLOR;
	case BlendMode::SrcAlpha:
		return GL_SRC_ALPHA;
	case BlendMode::OneMinusSrcAlpha:
		return GL_ONE_MINUS_SRC_ALPHA;
	case BlendMode::DstAlpha:
		return GL_DST_ALPHA;
	case BlendMode::OneMinusDstAlpha:
		return GL_ONE_MINUS_DST_ALPHA;
	case BlendMode::ConstantColor:
		return GL_CONSTANT_COLOR;
	case BlendMode::OneMinusConstantColor:
		return GL_ONE_MINUS_CONSTANT_COLOR;
	case BlendMode::ConstantAlpha:
		return GL_CONSTANT_ALPHA;
	case BlendMode::OneMinusConstantAlpha:
		return GL_ONE_MINUS_CONSTANT_ALPHA;
	case BlendMode::SrcAlphaSaturate:
		return GL_SRC_ALPHA_SATURATE;
	case BlendMode::Src1Color:
		return GL_SRC1_COLOR;
	case BlendMode::OneMinusSrc1Color:
		return GL_ONE_MINUS_SRC1_COLOR;
	case BlendMode::Src1Alpha:
		return GL_SRC1_ALPHA;
	case BlendMode::OneMinusSrc1Alpha:
		return GL_ONE_MINUS_SRC1_ALPHA;
	}
}
GLenum blendOp(BlendOp op)
{
	switch (op)
	{
	default:
	case BlendOp::Add:
		return GL_FUNC_ADD;
	case BlendOp::Substract:
		return GL_FUNC_SUBTRACT;
	case BlendOp::ReverseSubstract:
		return GL_FUNC_REVERSE_SUBTRACT;
	case BlendOp::Min:
		return GL_MIN;
	case BlendOp::Max:
		return GL_MAX;
	}
}

GLenum stencilFunc(StencilMode mode)
{
	switch (mode)
	{
	default:
	case StencilMode::None:
	case StencilMode::Never:
		return GL_NEVER;
	case StencilMode::Less:
		return GL_LESS;
	case StencilMode::LessOrEqual:
		return GL_LEQUAL;
	case StencilMode::Greater:
		return GL_GREATER;
	case StencilMode::GreaterOrEqual:
		return GL_GEQUAL;
	case StencilMode::Equal:
		return GL_EQUAL;
	case StencilMode::NotEqual:
		return GL_NOTEQUAL;
	case StencilMode::Always:
		return GL_ALWAYS;
	}
}

GLenum stencilOp(StencilOp op)
{
	switch (op)
	{
	default:
	case StencilOp::Keep:
		return GL_KEEP;
	case StencilOp::Zero:
		return GL_ZERO;
	case StencilOp::Replace:
		return GL_REPLACE;
	case StencilOp::Increment:
		return GL_INCR;
	case StencilOp::IncrementWrap:
		return GL_INCR_WRAP;
	case StencilOp::Decrement:
		return GL_DECR;
	case StencilOp::DecrementWrap:
		return GL_DECR_WRAP;
	case StencilOp::Invert:
		return GL_INVERT;
	}
}

GLenum primitive(PrimitiveType type)
{
	switch (type)
	{
	case PrimitiveType::Points:
		return GL_POINTS;
	case PrimitiveType::LineLoop:
		return GL_LINE_LOOP;
	case PrimitiveType::LineStrip:
		return GL_LINE_STRIP;
	case PrimitiveType::Lines:
		return GL_LINES;
	case PrimitiveType::TriangleStrip:
		return GL_TRIANGLE_STRIP;
	case PrimitiveType::TriangleFan:
		return GL_TRIANGLE_FAN;
	default:
	case PrimitiveType::Triangles:
		return GL_TRIANGLES;
	}
}

};

class GLTexture : public Texture
{
public:
	GLTexture(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureComponent component, TextureFlag flags,
		Sampler sampler, 
		void* data
	) :
		Texture(width, height, TextureType::Texture2D, format, component, flags, sampler),
		m_copyFBO(0),
		m_textureID(0)
	{
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, gl::componentInternal(m_component), width, height, 0, gl::component(m_component), gl::format(m_format), data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl::filter(m_sampler.filterMag, Sampler::MipMapMode::None));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl::filter(m_sampler.filterMin, m_sampler.mipmapMode));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl::wrap(m_sampler.wrapU));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl::wrap(m_sampler.wrapV));
		if (m_sampler.mipmapMode != Sampler::MipMapMode::None && data != nullptr)
			glGenerateMipmap(GL_TEXTURE_2D);
	}
	GLTexture(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureComponent component, TextureFlag flags,
		Sampler sampler,
		void* data,
		uint8_t samples
	) :
		Texture(width, height, TextureType::Texture2DMultisample, format, component, flags, sampler),
		m_copyFBO(0),
		m_textureID(0)
	{
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_textureID);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, gl::componentInternal(m_component), width, height, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, gl::filter(m_sampler.filterMag, Sampler::MipMapMode::None));
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, gl::filter(m_sampler.filterMin, m_sampler.mipmapMode));
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, gl::wrap(m_sampler.wrapU));
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, gl::wrap(m_sampler.wrapV));
		//if (m_sampler.mipmapMode != Sampler::MipMapMode::None && data != nullptr)
		//	glGenerateMipmap(GL_TEXTURE_2D_MULTISAMPLE);
	}
	GLTexture(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureComponent component, TextureFlag flags,
		Sampler sampler,
		void* px, void* nx,
		void* py, void* ny,
		void* pz, void* nz
	) :
		Texture(width, height, TextureType::TextureCubemap, format, component, flags, sampler),
		m_copyFBO(0),
		m_textureID(0)
	{
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, gl::componentInternal(m_component), width, height, 0, gl::component(m_component), gl::format(m_format), px);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, gl::componentInternal(m_component), width, height, 0, gl::component(m_component), gl::format(m_format), nx);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, gl::componentInternal(m_component), width, height, 0, gl::component(m_component), gl::format(m_format), py);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, gl::componentInternal(m_component), width, height, 0, gl::component(m_component), gl::format(m_format), ny);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, gl::componentInternal(m_component), width, height, 0, gl::component(m_component), gl::format(m_format), pz);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, gl::componentInternal(m_component), width, height, 0, gl::component(m_component), gl::format(m_format), nz);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, gl::filter(m_sampler.filterMag, Sampler::MipMapMode::None));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, gl::filter(m_sampler.filterMin, m_sampler.mipmapMode));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, gl::wrap(m_sampler.wrapU));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, gl::wrap(m_sampler.wrapV));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, gl::wrap(m_sampler.wrapW));
		if (m_sampler.mipmapMode != Sampler::MipMapMode::None && px != nullptr)
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	GLTexture(GLTexture&) = delete;
	GLTexture& operator=(GLTexture&) = delete;
	~GLTexture()
	{
		if (m_textureID != 0)
			glDeleteTextures(1, &m_textureID);
		if (m_copyFBO != 0)
			glDeleteFramebuffers(1, &m_copyFBO);
	}
	void upload(const void* data) override
	{
		upload(Rect{ 0, 0, m_width, m_height }, data);
	}
	void upload(const Rect& rect, const void* data) override
	{
		upload(0, rect, data);
		if (m_sampler.mipmapMode != Sampler::MipMapMode::None)
			glGenerateMipmap(gl::type(m_type));
	}
	void upload(uint32_t mipLevel, const Rect& rect, const void* data) override
	{
		AKA_ASSERT(rect.x + rect.w <= m_width, "");
		AKA_ASSERT(rect.y + rect.h <= m_height, "");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(gl::type(m_type), m_textureID);
		glTexSubImage2D(gl::type(m_type), mipLevel, (GLint)rect.x, (GLint)rect.y, (GLsizei)rect.w, (GLsizei)rect.h, gl::component(m_component), gl::format(m_format), data);
	}
	void download(void* data) override
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(gl::type(m_type), m_textureID);
		glGetTexImage(gl::type(m_type), 0, gl::component(m_component), gl::format(m_format), data);
	}
	void copy(Texture::Ptr src, const Rect& rect) override
	{
		AKA_ASSERT(src->format() == this->format(), "Invalid format");
		AKA_ASSERT(src->component() == this->component(), "Invalid components");
		AKA_ASSERT(rect.x + rect.w < src->width() || rect.y + rect.h < src->height(), "Rect not in range");
		AKA_ASSERT(rect.x + rect.w < this->width() || rect.y + rect.h < this->height(), "Rect not in range");
		if (m_copyFBO == 0)
			glGenFramebuffers(1, &m_copyFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_copyFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gl::type(m_type), reinterpret_cast<GLTexture*>(src.get())->getTextureID(), 0);
		glBindTexture(gl::type(m_type), m_textureID);
		glCopyTexImage2D(gl::type(m_type), 0, gl::componentInternal(m_component), rect.x, rect.y, rect.w, rect.h, 0);
		// TODO copy all mip map level
		glBindTexture(gl::type(m_type), 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	GLuint getTextureID() const
	{
		return m_textureID;
	}
	Handle handle() const override
	{
		return Handle(static_cast<uintptr_t>(m_textureID));
	}
private:
	GLuint m_copyFBO;
	GLuint m_textureID;
};

class GLShader : public Shader
{
public:
	GLShader(ShaderID vertex, ShaderID fragment, ShaderID compute, const std::vector<Attributes>& attributes) :
		Shader(attributes)
	{
		GLuint vert = static_cast<GLuint>(vertex.value());
		GLuint frag = static_cast<GLuint>(fragment.value());
		GLuint comp = static_cast<GLuint>(compute.value());
		m_programID = glCreateProgram();
		// Attach shaders
		if (vert != 0 && glIsShader(vert) == GL_TRUE)
			glAttachShader(m_programID, vert);
		if (frag != 0 && glIsShader(frag) == GL_TRUE)
			glAttachShader(m_programID, frag);
		if (comp != 0 && glIsShader(comp) == GL_TRUE)
			glAttachShader(m_programID, comp);

		// link program
		glLinkProgram(m_programID);
		GLint linked;
		glGetProgramiv(m_programID, GL_LINK_STATUS, &linked);
		if (linked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> errorLog(maxLength);
			glGetProgramInfoLog(m_programID, maxLength, &maxLength, &errorLog[0]);
			std::string str(errorLog.begin(), errorLog.end());
			// Exit with failure.
			glDeleteProgram(m_programID); // Don't leak the program.
			if (vert != 0)
				glDeleteShader(vert);
			if (frag != 0)
				glDeleteShader(frag);
			if (comp != 0)
				glDeleteShader(comp);
			Logger::error("[GL] ", str);
		}
		else
		{
			// Always detach shaders after a successful link.
			if (vert != 0)
			{
				glDetachShader(m_programID, vert);
				glDeleteShader(vert);
			}
			if (frag != 0)
			{
				glDetachShader(m_programID, frag);
				glDeleteShader(frag);
			}
			if (comp != 0)
			{
				glDetachShader(m_programID, comp);
				glDeleteShader(comp);
			}
			glValidateProgram(m_programID);
			GLint status = 0;
			glGetProgramiv(m_programID, GL_VALIDATE_STATUS, &status);
			if (status != GL_TRUE)
				Logger::error("[GL] Program is not valid");
			else
				m_valid = true;
		}
	}
	GLShader(const GLShader&) = delete;
	GLShader& operator=(const GLShader&) = delete;
	~GLShader()
	{
		if (m_programID != 0)
			glDeleteProgram(m_programID);
	}
public:
	GLuint getProgramID() const { return m_programID; }
private:
	GLuint m_programID;
};

class GLShaderMaterial : public ShaderMaterial
{
public:
	GLShaderMaterial(Shader::Ptr shader) :
		ShaderMaterial(shader)
	{
		GLShader* glShader = reinterpret_cast<GLShader*>(m_shader.get());
		GLint activeUniforms = 0;
		uint32_t bufferSize = 0;
		uint32_t textureCount = 0;
		uint32_t imageCount = 0;
		glGetProgramiv(glShader->getProgramID(), GL_ACTIVE_UNIFORMS, &activeUniforms);
		for (GLint iUniform = 0; iUniform < activeUniforms; iUniform++)
		{
			GLsizei length;
			GLsizei size;
			GLenum type;
			GLchar name[257];
			glGetActiveUniform(glShader->getProgramID(), iUniform, 256, &length, &size, &type, name);
			name[length] = '\0';

			Uniform uniform;
			uniform.id = UniformID(glGetUniformLocation(glShader->getProgramID(), name));
			uniform.name = name;
			uniform.bufferIndex = 0;
			uniform.arrayLength = size;
			switch (type)
			{
			case GL_IMAGE_2D:
				uniform.type = UniformType::Image2D;
				uniform.shaderType = ShaderType::Compute;
				imageCount += 1 * uniform.arrayLength;
				break;
			case GL_SAMPLER_2D:
				uniform.type = UniformType::Texture2D;
				uniform.shaderType = ShaderType::Fragment;
				// TODO add sampler
				textureCount += 1 * uniform.arrayLength;
				break;
			case GL_SAMPLER_CUBE:
				uniform.type = UniformType::TextureCubemap;
				uniform.shaderType = ShaderType::Fragment;
				textureCount += 1 * uniform.arrayLength;
				break;
			case GL_SAMPLER_2D_MULTISAMPLE:
				uniform.type = UniformType::Texture2DMultisample;
				uniform.shaderType = ShaderType::Fragment;
				textureCount += 1 * uniform.arrayLength;
				break;
			case GL_FLOAT:
				uniform.type = UniformType::Float;
				uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
				bufferSize += 1 * uniform.arrayLength;
				break;
			case GL_INT:
				uniform.type = UniformType::Int;
				uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
				bufferSize += 1 * uniform.arrayLength;
				break;
			case GL_UNSIGNED_INT:
				uniform.type = UniformType::UnsignedInt;
				uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
				bufferSize += 1 * uniform.arrayLength;
				break;
			case GL_FLOAT_VEC2:
				uniform.type = UniformType::Vec2;
				uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
				bufferSize += 2 * uniform.arrayLength;
				break;
			case GL_FLOAT_VEC3:
				uniform.type = UniformType::Vec3;
				uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
				bufferSize += 3 * uniform.arrayLength;
				break;
			case GL_FLOAT_VEC4:
				uniform.type = UniformType::Vec4;
				uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
				bufferSize += 4 * uniform.arrayLength;
				break;
			case GL_FLOAT_MAT3:
				uniform.type = UniformType::Mat3;
				uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
				bufferSize += 9 * uniform.arrayLength;
				break;
			case GL_FLOAT_MAT4:
				uniform.type = UniformType::Mat4;
				uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
				bufferSize += 16 * uniform.arrayLength;
				break;
			default:
				Logger::warn("[GL] Unsupported Uniform Type : ", name);
				break;
			}
			m_uniforms.push_back(uniform);
		}
		m_data.resize(bufferSize, 0.f);
		m_textures.resize(textureCount, nullptr);
		m_images.resize(imageCount, nullptr);
	}
	GLShaderMaterial(const GLShaderMaterial&) = delete;
	GLShaderMaterial& operator=(const GLShaderMaterial&) = delete;
	~GLShaderMaterial()
	{
	}
public:
	void use() const
	{
		GLShader* glShader = reinterpret_cast<GLShader*>(m_shader.get());
		GLint textureUnit = 0;
		GLint imageUnit = 0;
		size_t offset = 0;
		glUseProgram(glShader->getProgramID());
		for (uint32_t iUniform = 0; iUniform < m_uniforms.size(); iUniform++)
		{
			const Uniform& uniform = m_uniforms[iUniform];
			switch (uniform.type)
			{
			default:
			case UniformType::None:
				Logger::error("[GL] Unsupported uniform type : ", (int)uniform.type, "(", uniform.name, ")");
				break;
			case UniformType::Texture2D: {
				std::vector<GLint> units;
				// Bind texture to units.
				for (uint32_t i = 0; i < uniform.arrayLength; i++)
				{
					GLint unit = textureUnit++;
					Texture::Ptr texture = m_textures[unit];
					GLTexture* glTexture = reinterpret_cast<GLTexture*>(texture.get());
					glActiveTexture(GL_TEXTURE0 + unit);
					if (texture != nullptr)
						glBindTexture(GL_TEXTURE_2D, glTexture->getTextureID());
					else
						glBindTexture(GL_TEXTURE_2D, 0);
					units.push_back(unit);
				}
				// Upload texture unit array.
				glUniform1iv((GLint)uniform.id.value(), (GLsizei)units.size(), units.data());
				break;
			}
			case UniformType::Texture2DMultisample: {
				std::vector<GLint> units;
				// Bind texture to units.
				for (uint32_t i = 0; i < uniform.arrayLength; i++)
				{
					GLint unit = textureUnit++;
					Texture::Ptr texture = m_textures[unit];
					GLTexture* glTexture = reinterpret_cast<GLTexture*>(texture.get());
					glActiveTexture(GL_TEXTURE0 + unit);
					if (texture != nullptr)
						glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, glTexture->getTextureID());
					else
						glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
					units.push_back(unit);
				}
				// Upload texture unit array.
				glUniform1iv((GLint)uniform.id.value(), (GLsizei)units.size(), units.data());
				break;
			}
			case UniformType::TextureCubemap: {
				std::vector<GLint> units;
				// Bind texture to units.
				for (uint32_t i = 0; i < uniform.arrayLength; i++)
				{
					GLint unit = textureUnit++;
					Texture::Ptr texture = m_textures[unit];
					GLTexture* glTexture = reinterpret_cast<GLTexture*>(texture.get());
					glActiveTexture(GL_TEXTURE0 + unit);
					if (texture != nullptr)
						glBindTexture(GL_TEXTURE_CUBE_MAP, glTexture->getTextureID());
					else
						glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
					units.push_back(unit);
				}
				// Upload texture unit array.
				glUniform1iv((GLint)uniform.id.value(), (GLsizei)units.size(), units.data());
				break;
			}
			case UniformType::Image2D: {
				std::vector<GLint> units;
				// Bind images to units.
				for (uint32_t i = 0; i < uniform.arrayLength; i++)
				{
					GLint unit = imageUnit++;
					Texture::Ptr image = m_images[unit];
					GLTexture* glImage = reinterpret_cast<GLTexture*>(image.get());
					if (image != nullptr)
						glBindImageTexture(0, glImage->getTextureID(), 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);// gl::format(image->format()));
					else
						glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);//gl::format(image->format()));
					units.push_back(unit);
				}
				// Upload texture unit array.
				glUniform1iv((GLint)uniform.id.value(), (GLsizei)units.size(), units.data());
				break;
			}
			case UniformType::Sampler2D: 
			case UniformType::SamplerCube: {
				// TODO store sampler
				break;
			}
			case UniformType::Mat4: {
				glUniformMatrix4fv((GLint)uniform.id.value(), (GLsizei)uniform.arrayLength, false, &m_data[offset]);
				offset += 16 * uniform.arrayLength;
				break;
			}
			case UniformType::Mat3: {
				glUniformMatrix3fv((GLint)uniform.id.value(), (GLsizei)uniform.arrayLength, false, &m_data[offset]);
				offset += 9 * uniform.arrayLength;
				break;
			}
			case UniformType::Float: {
				glUniform1fv((GLint)uniform.id.value(), (GLsizei)uniform.arrayLength, &m_data[offset]);
				offset += uniform.arrayLength;
				break;
			}
			case UniformType::Int: {
				glUniform1iv((GLint)uniform.id.value(), (GLsizei)uniform.arrayLength, reinterpret_cast<const int*>(&m_data[offset]));
				offset += uniform.arrayLength;
				break;
			}
			case UniformType::UnsignedInt: {
				glUniform1uiv((GLint)uniform.id.value(), (GLsizei)uniform.arrayLength, reinterpret_cast<const unsigned int*>(&m_data[offset]));
				offset += uniform.arrayLength;
				break;
			}
			case UniformType::Vec3: {
				glUniform3fv((GLint)uniform.id.value(), (GLsizei)uniform.arrayLength, &m_data[offset]);
				offset += 3 * uniform.arrayLength;
				break;
			}
			case UniformType::Vec4: {
				glUniform4fv((GLint)uniform.id.value(), (GLsizei)uniform.arrayLength, &m_data[offset]);
				offset += 4 * uniform.arrayLength;
				break;
			}
			}
		}
		AKA_ASSERT(textureUnit < GL_MAX_TEXTURE_UNITS - GL_TEXTURE0, "Too much textures");
	}
};

class GLBuffer : public Buffer
{
	static GLenum type(BufferType type) {
		switch (type) {
		case BufferType::Array:
			return GL_ARRAY_BUFFER;
		case BufferType::ElementArray:
			return GL_ELEMENT_ARRAY_BUFFER;
		default:
			return 0;
		}
	}
	static GLenum access(BufferUsage usage, BufferAccess access) {
		switch (usage) {
		case BufferUsage::Static:
			return (access == BufferAccess::ReadOnly ? GL_STATIC_READ : (access == BufferAccess::WriteOnly ? GL_STATIC_DRAW : GL_STATIC_COPY));
		case BufferUsage::Stream:
			return (access == BufferAccess::ReadOnly ? GL_STREAM_READ : (access == BufferAccess::WriteOnly ? GL_STREAM_DRAW : GL_STREAM_COPY));
		case BufferUsage::Dynamic:
			return (access == BufferAccess::ReadOnly ? GL_DYNAMIC_READ : (access == BufferAccess::WriteOnly ? GL_DYNAMIC_DRAW : GL_DYNAMIC_COPY));
		default:
			return 0;
		}
	}
public:
	GLBuffer(BufferType type, size_t size, BufferUsage usage, BufferAccess access) :
		Buffer(type, size, usage, access),
		m_bufferID(0)
	{
		glGenBuffers(1, &m_bufferID);
		
		glBindBuffer(GLBuffer::type(type), m_bufferID);
		glBufferData(GLBuffer::type(type), size, nullptr, GLBuffer::access(usage, access));
		glBindBuffer(GLBuffer::type(type), 0);
	}
	GLBuffer(const GLBuffer&) = delete;
	GLBuffer& operator=(const GLBuffer&) = delete;
	~GLBuffer()
	{
		if (m_bufferID)
			glDeleteBuffers(1, &m_bufferID);
	}

	void upload(const void* data, size_t size, size_t offset = 0) override
	{
		glBindBuffer(GLBuffer::type(m_type), m_bufferID);
		glBufferSubData(GLBuffer::type(m_type), offset, size, data);
		glBindBuffer(GLBuffer::type(m_type), 0);
	}

	void upload(const void* data) override
	{
		glBindBuffer(GLBuffer::type(m_type), m_bufferID);
		glBufferData(GLBuffer::type(m_type), m_size, data, GLBuffer::access(m_usage, m_access));
		glBindBuffer(GLBuffer::type(m_type), 0);
	}

	void download(void* data, size_t size, size_t offset = 0) override
	{
		glBindBuffer(GLBuffer::type(m_type), m_bufferID);
		glGetBufferSubData(GLBuffer::type(m_type), offset, size, data);
		glBindBuffer(GLBuffer::type(m_type), 0);
	}

	void download(void* data) override
	{
		glBindBuffer(GLBuffer::type(m_type), m_bufferID);
		glGetBufferSubData(GLBuffer::type(m_type), 0, m_size, data);
		glBindBuffer(GLBuffer::type(m_type), 0);
	}

	void* map(BufferAccess access) override
	{
		glBindBuffer(GLBuffer::type(m_type), m_bufferID);
		GLenum glAccess = GL_READ_ONLY;
		switch (access)
		{
		case BufferAccess::ReadOnly:
			AKA_ASSERT(m_access == BufferAccess::ReadOnly || m_access == BufferAccess::ReadAndWrite, "");
			glAccess = GL_READ_ONLY;
			break;
		case BufferAccess::WriteOnly:
			AKA_ASSERT(m_access == BufferAccess::WriteOnly || m_access == BufferAccess::ReadAndWrite, "");
			glAccess = GL_WRITE_ONLY;
			break;
		default:
		case BufferAccess::ReadAndWrite:
			AKA_ASSERT(m_access == BufferAccess::ReadAndWrite, "");
			glAccess = GL_READ_WRITE;
			break;
		}
		void* data = glMapBuffer(GLBuffer::type(m_type), glAccess);
		glBindBuffer(GLBuffer::type(m_type), 0);
		return data;
	}

	void unmap() override 
	{
		glBindBuffer(GLBuffer::type(m_type), m_bufferID);
		glUnmapBuffer(GLBuffer::type(m_type));
		glBindBuffer(GLBuffer::type(m_type), 0);
	}
private:
	GLuint m_bufferID;
};

class GLMesh : public Mesh
{
public:
	GLMesh() :
		Mesh(),
		m_vao(0),
		m_vertexVbo(0),
		m_indexVbo(0)
	{
		glGenVertexArrays(1, &m_vao);
	}
	GLMesh(const GLMesh&) = delete;
	GLMesh& operator=(const GLMesh&) = delete;
	~GLMesh()
	{
		if (m_vao)
			glDeleteVertexArrays(1, &m_vao);
		if (m_vertexVbo)
			glDeleteBuffers(1, &m_vertexVbo);
		if (m_indexVbo)
			glDeleteBuffers(1, &m_indexVbo);
	}
public:
	void vertices(const VertexData& vertex, const void* vertices, size_t count) override
	{
		m_vertexData = vertex;
		m_vertexCount = static_cast<uint32_t>(count);
		m_vertexStride = vertex.stride();
		glBindVertexArray(m_vao);
		if (m_vertexVbo == 0)
			glGenBuffers(1, &m_vertexVbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexVbo);
		size_t offset = 0;
		for (const VertexData::Attribute& attribute : vertex.attributes)
		{
			GLint componentSize = size(attribute.format);
			GLint components = size(attribute.type);
			GLenum type = gl::format(attribute.format);
			GLboolean normalized = GL_FALSE;
			glEnableVertexAttribArray(attribute.index);
			glVertexAttribPointer(attribute.index, components, type, normalized, m_vertexStride, (void*)offset);
			offset += components * componentSize;
		}

		glBufferData(GL_ARRAY_BUFFER, m_vertexStride * count, vertices, GL_DYNAMIC_DRAW);
		// Do not unbind buffers as they will be unbind from VAO
		glBindVertexArray(0);
	}

	void indices(IndexFormat indexFormat, const void* indices, size_t count) override
	{
		glBindVertexArray(m_vao);
		if (m_indexVbo == 0)
			glGenBuffers(1, &m_indexVbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVbo);
		m_indexFormat = indexFormat;
		m_indexSize = size(indexFormat);
		m_indexCount = (uint32_t)count;
		// GL_DYNAMIC_DRAW so we can change buffer data
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexSize * count, indices, GL_DYNAMIC_DRAW);
		glBindVertexArray(0);
	}
	void draw(PrimitiveType type, uint32_t indexCount, uint32_t indexOffset) const override
	{
		glBindVertexArray(m_vao);
		// We could use glDrawArrays for non indexed array.
		// But using indexed allow to reduce the number of draw call for different types
		// We can also reuse vertices with glDrawElements as they are indexed
		GLenum primitive = gl::primitive(type);
		if (m_indexCount > 0)
		{
			void* indices = (void*)(uintptr_t)(m_indexSize * indexOffset);
			glDrawElements(
				primitive,
				static_cast<GLsizei>(indexCount),
				gl::format(m_indexFormat),
				indices
			);
		}
		else
		{
			glDrawArrays(primitive, 0, m_vertexCount);
		}
		glBindVertexArray(0);
	}

private:
	GLuint m_vao;
	GLuint m_vertexVbo;
	GLuint m_indexVbo;
};

class GLFramebuffer : public Framebuffer
{
public:
	GLFramebuffer(FramebufferAttachment* attachments, size_t count) :
		Framebuffer(attachments, count),
		m_framebufferID(0)
	{
		glGenFramebuffers(1, &m_framebufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
		std::vector<GLenum> drawBuffers;
		for (size_t iAtt = 0; iAtt < count; iAtt++)
		{
			// TODO assert uniqueness of attachment
			// TODO if texture nullptr, create render target instead ?
			GLTexture* glTexture = reinterpret_cast<GLTexture*>(attachments[iAtt].texture.get());
			GLenum type = gl::attachmentType(attachments[iAtt].type);
			glFramebufferTexture2D(GL_FRAMEBUFFER, type, GL_TEXTURE_2D, glTexture->getTextureID(), 0);
			if (type >= GL_COLOR_ATTACHMENT0 && type <= GL_COLOR_ATTACHMENT15)
				drawBuffers.push_back(type);
		}
		AKA_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not created");
		glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	GLFramebuffer(const GLFramebuffer&) = delete;
	GLFramebuffer& operator=(const GLFramebuffer&) = delete;
	~GLFramebuffer()
	{
		if (m_framebufferID != 0)
			glDeleteFramebuffers(1, &m_framebufferID);
	}
	void resize(uint32_t width, uint32_t height) override
	{
		if (width = m_width && height == m_height)
			return;
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
		for (FramebufferAttachment& attachment : m_attachments)
		{
			std::shared_ptr<GLTexture> tex = std::make_shared<GLTexture>(
				width, height, 
				attachment.texture->format(), 
				attachment.texture->component(), 
				TextureFlag::RenderTarget,
				attachment.texture->sampler(),
				nullptr
			);
			glFramebufferTexture2D(GL_FRAMEBUFFER, gl::attachmentType(attachment.type), GL_TEXTURE_2D, tex->getTextureID(), 0);
			attachment.texture = tex;
		}
		AKA_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not created");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_width = width;
		m_height = height;
	}
	void clear(const color4f& color, float depth, int stencil, ClearMask mask) override
	{
		if (mask == ClearMask::None)
			return;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebufferID);
		GLenum glMask = 0;
		if ((mask & ClearMask::Color) == ClearMask::Color)
		{
			glClearColor(color.r, color.g, color.b, color.a);
			glMask |= GL_COLOR_BUFFER_BIT;
		}
		if ((mask & ClearMask::Depth) == ClearMask::Depth)
		{
			glDepthMask(true); // Ensure we can correctly clear the depth buffer.
			glClearDepth(depth);
			glMask |= GL_DEPTH_BUFFER_BIT;
		}
		if ((mask & ClearMask::Stencil) == ClearMask::Stencil)
		{
			glClearStencil(stencil);
			glMask |= GL_STENCIL_BUFFER_BIT;
		}
		glClear(glMask);
	}
	void blit(Framebuffer::Ptr src, Rect rectSrc, Rect rectDst, FramebufferAttachmentType type, Sampler::Filter filter) override
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebufferID);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, reinterpret_cast<GLFramebuffer*>(src.get())->m_framebufferID);
		GLenum mask = 0;
		switch (type)
		{
		case aka::FramebufferAttachmentType::Color0:
		case aka::FramebufferAttachmentType::Color1:
		case aka::FramebufferAttachmentType::Color2:
		case aka::FramebufferAttachmentType::Color3:
			mask |= GL_COLOR_BUFFER_BIT;
			break;
		case aka::FramebufferAttachmentType::Depth:
			mask |= GL_DEPTH_BUFFER_BIT;
			break;
		case aka::FramebufferAttachmentType::Stencil:
			mask |= GL_STENCIL_BUFFER_BIT;
			break;
		case aka::FramebufferAttachmentType::DepthStencil:
			mask |= GL_DEPTH_BUFFER_BIT;
			mask |= GL_STENCIL_BUFFER_BIT;
			break;
		default:
			break;
		}
		glBlitFramebuffer(
			rectSrc.x, rectSrc.y, rectSrc.w, rectSrc.h, 
			rectDst.x, rectDst.y, rectDst.w, rectDst.h,
			mask,
			gl::filter(filter)
		);
	}
	void attachment(FramebufferAttachmentType type, Texture::Ptr texture) override
	{
		bool exist = false;
		for (FramebufferAttachment& attachment : m_attachments)
		{
			if (attachment.type == type)
			{
				if (attachment.texture == texture)
					return;
				attachment.texture = texture;
				exist = true;
				break;
			}
		}
		// Key does not exist yet.
		if (!exist)
			m_attachments.push_back(FramebufferAttachment{ type, texture });
		
		GLTexture* glTexture = reinterpret_cast<GLTexture*>(texture.get());
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, gl::attachmentType(type), GL_TEXTURE_2D, glTexture->getTextureID(), 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Recompute size
		m_width = m_attachments[0].texture->width();
		m_height = m_attachments[0].texture->height();

		for (size_t i = 1; i < m_attachments.size(); ++i)
		{
			if (m_width > m_attachments[i].texture->width())
				m_width = m_attachments[i].texture->width();
			if (m_height > m_attachments[i].texture->height())
				m_height = m_attachments[i].texture->height();
		}
	}
	GLuint getFramebufferID() const { return m_framebufferID; }
private:
	GLuint m_framebufferID;
};

class GLBackBuffer :
	public Framebuffer,
	EventListener<BackbufferResizeEvent>
{
public:
	GLBackBuffer(uint32_t width, uint32_t height) :
		Framebuffer(width, height)
	{
	}
	GLBackBuffer(const GLBackBuffer&) = delete;
	GLBackBuffer& operator=(const GLBackBuffer&) = delete;
	~GLBackBuffer()
	{
	}
	void resize(uint32_t width, uint32_t height) override
	{
		m_width = width;
		m_height = height;
	}
	void onReceive(const BackbufferResizeEvent& event) override
	{
		resize(event.width, event.height);
	}
	void clear(const color4f& color, float depth, int stencil, ClearMask mask) override
	{
		if (mask == ClearMask::None)
			return;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		GLenum glMask = 0;
		if ((mask & ClearMask::Color) == ClearMask::Color)
		{
			glClearColor(color.r, color.g, color.b, color.a);
			glMask |= GL_COLOR_BUFFER_BIT;
		}
		if ((mask & ClearMask::Depth) == ClearMask::Depth)
		{
			glDepthMask(true);
			glClearDepth(depth);
			glMask |= GL_DEPTH_BUFFER_BIT;
		}
		if ((mask & ClearMask::Stencil) == ClearMask::Stencil)
		{
			glClearStencil(stencil);
			glMask |= GL_STENCIL_BUFFER_BIT;
		}
		glClear(glMask);
	}
	void blit(Framebuffer::Ptr src, Rect rectSrc, Rect rectDst, FramebufferAttachmentType type, Sampler::Filter filter) override
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, reinterpret_cast<GLFramebuffer*>(src.get())->getFramebufferID());
		GLenum mask = 0;
		switch (type)
		{
		case aka::FramebufferAttachmentType::Color0:
		case aka::FramebufferAttachmentType::Color1:
		case aka::FramebufferAttachmentType::Color2:
		case aka::FramebufferAttachmentType::Color3:
			mask |= GL_COLOR_BUFFER_BIT;
			break;
		case aka::FramebufferAttachmentType::Depth:
			mask |= GL_DEPTH_BUFFER_BIT;
			break;
		case aka::FramebufferAttachmentType::Stencil:
			mask |= GL_STENCIL_BUFFER_BIT;
			break;
		case aka::FramebufferAttachmentType::DepthStencil:
			mask |= GL_DEPTH_BUFFER_BIT;
			mask |= GL_STENCIL_BUFFER_BIT;
			break;
		default:
			break;
		}
		glBlitFramebuffer(
			rectSrc.x, rectSrc.y, rectSrc.w, rectSrc.h,
			rectDst.x, rectDst.y, rectDst.w, rectDst.h,
			mask,
			gl::filter(filter)
		);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void attachment(FramebufferAttachmentType type, Texture::Ptr texture) override
	{
		Logger::error("Trying to set backbuffer attachement.");
	}
};

struct GLContext
{
	std::shared_ptr<GLBackBuffer> backbuffer = nullptr;
	bool vsync = true;
};

static GLContext gctx;

void GraphicBackend::initialize(uint32_t width, uint32_t height)
{
#if !defined(__APPLE__)
	glewExperimental = true; // Nécessaire dans le profil de base
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error("Could not init GLEW");
	}
#endif

#if defined(DEBUG)
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
	gctx.backbuffer = std::make_shared<GLBackBuffer>(width, height);
}

void GraphicBackend::destroy()
{
	gctx.backbuffer.reset();
	glFinish();
	glBindVertexArray(0);
}

GraphicApi GraphicBackend::api()
{
	return GraphicApi::OpenGL;
}

void GraphicBackend::frame()
{
}

void GraphicBackend::present()
{
#if defined(DEBUG)
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
		Logger::error("[GL] Error during frame : ", glGetErrorString(errorCode));
#endif
	glfwSwapBuffers(PlatformBackend::getGLFW3Handle());
}

Framebuffer::Ptr GraphicBackend::backbuffer()
{
	return gctx.backbuffer;
}

void GraphicBackend::render(RenderPass& pass)
{
	{
		// Set framebuffer
		if (pass.framebuffer != gctx.backbuffer)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, ((GLFramebuffer*)pass.framebuffer.get())->getFramebufferID());
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		// Clear
		if(pass.clear.mask != ClearMask::None)
			pass.framebuffer->clear(pass.clear.color, pass.clear.depth, pass.clear.stencil, pass.clear.mask);
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
			GLenum colorModeSrc = gl::blendMode(pass.blend.colorModeSrc);
			GLenum colorModeDst = gl::blendMode(pass.blend.colorModeDst);
			GLenum alphaModeSrc = gl::blendMode(pass.blend.alphaModeSrc);
			GLenum alphaModeDst = gl::blendMode(pass.blend.alphaModeDst);
			GLenum colorOp = gl::blendOp(pass.blend.colorOp);
			GLenum alphaOp = gl::blendOp(pass.blend.alphaOp);
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
			GLenum frontFunc = gl::stencilFunc(pass.stencil.front.mode);
			GLenum backFunc = gl::stencilFunc(pass.stencil.back.mode);
			glStencilFuncSeparate(GL_FRONT, frontFunc, 1, pass.stencil.readMask);
			glStencilFuncSeparate(GL_BACK, backFunc, 1, pass.stencil.readMask);
			glStencilOpSeparate(
				GL_FRONT,
				gl::stencilOp(pass.stencil.front.stencilFailed),
				gl::stencilOp(pass.stencil.front.stencilDepthFailed),
				gl::stencilOp(pass.stencil.front.stencilPassed)
			);
			glStencilOpSeparate(
				GL_BACK,
				gl::stencilOp(pass.stencil.back.stencilFailed),
				gl::stencilOp(pass.stencil.back.stencilDepthFailed),
				gl::stencilOp(pass.stencil.back.stencilPassed)
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
		GLShaderMaterial* material = (GLShaderMaterial*)pass.material.get();
		material->use();
	}
	{
		// Mesh
		pass.submesh.draw();
	}
}

void GraphicBackend::dispatch(ComputePass& pass)
{
	// TODO assert GL version is 4.3 at least (minimum requirement for compute)
	{
		// Shader
		GLShaderMaterial* material = (GLShaderMaterial*)pass.material.get();
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

void GraphicBackend::screenshot(const Path& path)
{
	glFinish();
	Image image(gctx.backbuffer->width(), gctx.backbuffer->height(), 4);
	glReadPixels(0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, image.bytes.data());
	image.save(path);
}

void GraphicBackend::vsync(bool enabled)
{
	if (enabled)
		glfwSwapInterval(1);
	else
		glfwSwapInterval(0);
	gctx.vsync = enabled;
}

Device GraphicBackend::getDevice(uint32_t id)
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

uint32_t GraphicBackend::deviceCount()
{
	return 0;
}

Texture::Ptr GraphicBackend::createTexture2D(
	uint32_t width, uint32_t height, 
	TextureFormat format, TextureComponent component, TextureFlag flags, 
	Sampler sampler, 
	void* data
)
{
	return std::make_shared<GLTexture>(width, height, format, component, flags, sampler, data);
}

Texture::Ptr GraphicBackend::createTexture2DMultisampled(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureComponent component, TextureFlag flags,
	Sampler sampler,
	void* data,
	uint8_t samples
)
{
	return std::make_shared<GLTexture>(width, height, format, component, flags, sampler, data, samples);
}

Texture::Ptr GraphicBackend::createTextureCubeMap(
	uint32_t width, uint32_t height, 
	TextureFormat format, TextureComponent component, TextureFlag flags,
	Sampler sampler, 
	void* px, void* nx,
	void* py, void* ny,
	void* pz, void* nz
)
{
	return std::make_shared<GLTexture>(width, height, format, component, flags, sampler, px, nx, py, ny, pz, nz);
}

Framebuffer::Ptr GraphicBackend::createFramebuffer(FramebufferAttachment* attachments, size_t count)
{
	return std::make_shared<GLFramebuffer>(attachments, count);
}

Buffer::Ptr GraphicBackend::createBuffer(BufferType type, size_t size, BufferUsage usage, BufferAccess access)
{
	return std::make_shared<GLBuffer>(type, size, usage, access);
}

Mesh::Ptr GraphicBackend::createMesh()
{
	return std::make_shared<GLMesh>();
}

ShaderID GraphicBackend::compile(const char* content, ShaderType type)
{
	GLuint shaderID = glCreateShader(gl::getType(type));
	glShaderSource(shaderID, 1, &content, NULL);
	glCompileShader(shaderID);
	GLint isCompiled;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);
		std::string str(errorLog.begin(), errorLog.end());
		// Exit with failure.
		glDeleteShader(shaderID); // Don't leak the shader.
		Logger::error("[GL] ", str);
		return ShaderID(0);
	}
	return ShaderID(shaderID);
}

Shader::Ptr GraphicBackend::createShader(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Attributes>& attributes)
{
	return std::make_shared<GLShader>(vert, frag, compute, attributes);
}

ShaderMaterial::Ptr aka::GraphicBackend::createShaderMaterial(Shader::Ptr shader)
{
	return std::make_shared<GLShaderMaterial>(shader);
}


};
#endif
