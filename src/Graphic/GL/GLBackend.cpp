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
GLenum attachmentType(AttachmentType type)
{
	switch (type)
	{
	default:
		Logger::warn("[GL] Framebuffer attachment type not defined : ", (int)type);
		return GL_COLOR_ATTACHMENT0;
	case AttachmentType::Color0:
		return GL_COLOR_ATTACHMENT0;
	case AttachmentType::Color1:
		return GL_COLOR_ATTACHMENT1;
	case AttachmentType::Color2:
		return GL_COLOR_ATTACHMENT2;
	case AttachmentType::Color3:
		return GL_COLOR_ATTACHMENT3;
	case AttachmentType::Depth:
		return GL_DEPTH_ATTACHMENT;
	case AttachmentType::Stencil:
		return GL_STENCIL_ATTACHMENT;
	case AttachmentType::DepthStencil:
		return GL_DEPTH_STENCIL_ATTACHMENT;
	}
}

GLenum filter(TextureFilter type, TextureMipMapMode mode = TextureMipMapMode::None)
{
	switch (mode)
	{
	case TextureMipMapMode::Linear:
		if (type == TextureFilter::Linear)
			return GL_LINEAR_MIPMAP_LINEAR; // trilinear
		else if (type == TextureFilter::Nearest)
			return GL_LINEAR_MIPMAP_NEAREST;
		break;
	case TextureMipMapMode::Nearest:
		if (type == TextureFilter::Linear)
			return GL_NEAREST_MIPMAP_LINEAR; // bilinear
		else if (type == TextureFilter::Nearest)
			return GL_NEAREST_MIPMAP_NEAREST;
		break;
	default:
	case TextureMipMapMode::None:
		if (type == TextureFilter::Linear)
			return GL_LINEAR;
		else if (type == TextureFilter::Nearest)
			return GL_NEAREST;
		break;
	}
	Logger::warn("[GL] Invalid filtering values : ", (int)type, " & ", (int)mode);
	return 0;
}

GLenum wrap(TextureWrap wrap) {
	switch (wrap) {
	default:
	case TextureWrap::Repeat:
		return GL_REPEAT;
	case TextureWrap::Mirror:
		return GL_MIRRORED_REPEAT;
	case TextureWrap::ClampToEdge:
		return GL_CLAMP_TO_EDGE;
	case TextureWrap::ClampToBorder:
		return GL_CLAMP_TO_BORDER;
	}
}

GLenum componentInternal(TextureFormat format) {
	switch (format) {
	default: throw std::runtime_error("Not implemneted");
	case TextureFormat::R8: return GL_R8;
	case TextureFormat::R8U: return GL_R8UI;
	case TextureFormat::R16: return GL_R16;
	case TextureFormat::R16U: return GL_R16UI;
	case TextureFormat::R16F: return GL_R16F;
	case TextureFormat::R32F: return GL_R32F;

	case TextureFormat::RG8: return GL_RG8;
	case TextureFormat::RG8U: return GL_RG8UI;
	case TextureFormat::RG16: return GL_RG16;
	case TextureFormat::RG16U: return GL_RG16UI;
	case TextureFormat::RG16F: return GL_RG16F;
	case TextureFormat::RG32F: return GL_RG32F;

	case TextureFormat::RGB8: return GL_RGB8;
	case TextureFormat::RGB8U: return GL_RGB8UI;
	case TextureFormat::RGB16: return GL_RGB16;
	case TextureFormat::RGB16U: return GL_RGB16UI;
	case TextureFormat::RGB16F: return GL_RGB16F;
	case TextureFormat::RGB32F: return GL_RGB32F;

	case TextureFormat::RGBA8: return GL_RGBA8;
	case TextureFormat::RGBA8U: return GL_RGBA8UI;
	case TextureFormat::RGBA16: return GL_RGBA16;
	case TextureFormat::RGBA16U: return GL_RGBA16UI;
	case TextureFormat::RGBA16F: return GL_RGBA16F;
	case TextureFormat::RGBA32F: return GL_RGBA32F;

	case TextureFormat::Depth: return GL_DEPTH_COMPONENT;
	case TextureFormat::Depth16: return GL_DEPTH_COMPONENT16;
	case TextureFormat::Depth24: return GL_DEPTH_COMPONENT24;
	case TextureFormat::Depth32: return GL_DEPTH_COMPONENT32;
	case TextureFormat::Depth32F: return GL_DEPTH_COMPONENT32F;
	case TextureFormat::DepthStencil: return GL_DEPTH_STENCIL;
	case TextureFormat::Depth0Stencil8: return GL_STENCIL_INDEX8;
	case TextureFormat::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
	case TextureFormat::Depth32FStencil8: return GL_DEPTH32F_STENCIL8;
	}
}

GLenum component(TextureFormat component) {
	switch (component) {
	default:
		throw std::runtime_error("Not implemneted");
	case TextureFormat::R8U:
	case TextureFormat::R16U:
		return GL_RED_INTEGER;
	case TextureFormat::R8:
	case TextureFormat::R16:
	case TextureFormat::R16F:
	case TextureFormat::R32F:
		return GL_RED;
	case TextureFormat::RG8U:
	case TextureFormat::RG16U:
		return GL_RG_INTEGER;
	case TextureFormat::RG8:
	case TextureFormat::RG16:
	case TextureFormat::RG16F:
	case TextureFormat::RG32F:
		return GL_RG;
	case TextureFormat::RGB8U:
	case TextureFormat::RGB16U:
		return GL_RGB_INTEGER;
	case TextureFormat::RGB8:
	case TextureFormat::RGB16:
	case TextureFormat::RGB16F:
	case TextureFormat::RGB32F:
		return GL_RGB;
	case TextureFormat::RGBA8U:
	case TextureFormat::RGBA16U:
		return GL_RGBA_INTEGER;
	case TextureFormat::RGBA8:
	case TextureFormat::RGBA16:
	case TextureFormat::RGBA16F:
	case TextureFormat::RGBA32F:
		return GL_RGBA;
	case TextureFormat::Depth:
	case TextureFormat::Depth16:
	case TextureFormat::Depth24:
	case TextureFormat::Depth32:
	case TextureFormat::Depth32F:
		return GL_DEPTH_COMPONENT;
	case TextureFormat::DepthStencil:
	case TextureFormat::Depth24Stencil8:
	case TextureFormat::Depth32FStencil8:
	case TextureFormat::Depth0Stencil8:
		return GL_DEPTH_STENCIL;
	}
}

GLenum type(TextureType type) {
	switch (type) {
	default: throw std::runtime_error("Not implemneted");
	case TextureType::Texture2D: return GL_TEXTURE_2D;
	case TextureType::TextureCubeMap: return GL_TEXTURE_CUBE_MAP;
	case TextureType::Texture2DMultisample: return GL_TEXTURE_2D_MULTISAMPLE;
	}
}

GLenum format(TextureFormat format) {
	switch (format) {
	default: throw std::runtime_error("Not implemneted");
	case TextureFormat::R8: return GL_UNSIGNED_BYTE;
	case TextureFormat::R8U: return GL_UNSIGNED_BYTE;
	case TextureFormat::R16: return GL_UNSIGNED_SHORT;
	case TextureFormat::R16U: return GL_UNSIGNED_SHORT;
	case TextureFormat::R16F: return GL_HALF_FLOAT;
	case TextureFormat::R32F: return GL_FLOAT;

	case TextureFormat::RG8:return GL_UNSIGNED_BYTE;
	case TextureFormat::RG16:return GL_UNSIGNED_SHORT;
	case TextureFormat::RG8U: return GL_UNSIGNED_BYTE;
	case TextureFormat::RG16U: return GL_UNSIGNED_SHORT;
	case TextureFormat::RG16F: return GL_HALF_FLOAT;
	case TextureFormat::RG32F: return GL_FLOAT;

	case TextureFormat::RGB8:return GL_UNSIGNED_BYTE;
	case TextureFormat::RGB8U: return GL_UNSIGNED_BYTE;
	case TextureFormat::RGB16:return GL_UNSIGNED_SHORT;
	case TextureFormat::RGB16U: return GL_UNSIGNED_SHORT;
	case TextureFormat::RGB16F: return GL_HALF_FLOAT;
	case TextureFormat::RGB32F: return GL_FLOAT;

	case TextureFormat::RGBA8:return GL_UNSIGNED_BYTE;
	case TextureFormat::RGBA8U: return GL_UNSIGNED_BYTE;
	case TextureFormat::RGBA16:return GL_UNSIGNED_SHORT;
	case TextureFormat::RGBA16U: return GL_UNSIGNED_SHORT;
	case TextureFormat::RGBA16F: return GL_HALF_FLOAT;
	case TextureFormat::RGBA32F: return GL_FLOAT;

	case TextureFormat::Depth: return GL_FLOAT;
	case TextureFormat::Depth16: return GL_FLOAT; // ?
	case TextureFormat::Depth24: return GL_FLOAT; // ?
	case TextureFormat::Depth32: return GL_FLOAT; // ?
	case TextureFormat::Depth32F: return GL_FLOAT;

	case TextureFormat::DepthStencil: return GL_UNSIGNED_INT_24_8;
	case TextureFormat::Depth24Stencil8: return GL_UNSIGNED_INT_24_8;
	case TextureFormat::Depth32FStencil8: return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
	case TextureFormat::Depth0Stencil8: return GL_UNSIGNED_BYTE;
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
	/*case ShaderType::TessControl:
		return GL_TESS_CONTROL_SHADER;
	case ShaderType::TessEvaluation:
		return GL_TESS_EVALUATION_SHADER;*/
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

GLenum stencilCompare(StencilCompare mode)
{
	switch (mode)
	{
	default:
	case StencilCompare::None:
	case StencilCompare::Never:
		return GL_NEVER;
	case StencilCompare::Less:
		return GL_LESS;
	case StencilCompare::LessOrEqual:
		return GL_LEQUAL;
	case StencilCompare::Greater:
		return GL_GREATER;
	case StencilCompare::GreaterOrEqual:
		return GL_GEQUAL;
	case StencilCompare::Equal:
		return GL_EQUAL;
	case StencilCompare::NotEqual:
		return GL_NOTEQUAL;
	case StencilCompare::Always:
		return GL_ALWAYS;
	}
}

GLenum stencilMode(StencilMode op)
{
	switch (op)
	{
	default:
	case StencilMode::Keep:
		return GL_KEEP;
	case StencilMode::Zero:
		return GL_ZERO;
	case StencilMode::Replace:
		return GL_REPLACE;
	case StencilMode::Increment:
		return GL_INCR;
	case StencilMode::IncrementWrap:
		return GL_INCR_WRAP;
	case StencilMode::Decrement:
		return GL_DECR;
	case StencilMode::DecrementWrap:
		return GL_DECR_WRAP;
	case StencilMode::Invert:
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

class GLTexture2D : public Texture2D
{
public:
	GLTexture2D(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* data) :
		Texture2D(width, height, format, flags),
		m_textureID(0)
	{
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, gl::componentInternal(m_format), width, height, 0, gl::component(m_format), gl::format(m_format), data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.f);
		if ((TextureFlag::GenerateMips & flags) == TextureFlag::GenerateMips)
			glGenerateMipmap(GL_TEXTURE_2D);
	}
	~GLTexture2D()
	{
		if (m_textureID != 0)
			glDeleteTextures(1, &m_textureID);
	}
	void upload(const void* data, uint32_t level) override
	{
		upload(Rect{ 0, 0, m_width, m_height }, data, level);
	}
	void upload(const Rect& rect, const void* data, uint32_t level) override
	{
		AKA_ASSERT(rect.x + rect.w <= m_width, "");
		AKA_ASSERT(rect.y + rect.h <= m_height, "");
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexSubImage2D(GL_TEXTURE_2D, level, (GLint)rect.x, (GLint)rect.y, (GLsizei)rect.w, (GLsizei)rect.h, gl::component(m_format), gl::format(m_format), data);
		if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
			glGenerateMipmap(GL_TEXTURE_2D);
	}
	void download(void* data, uint32_t level) override
	{
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glGetTexImage(GL_TEXTURE_2D, level, gl::component(m_format), gl::format(m_format), data);
	}
	void copy(const Texture2D::Ptr& dst, const Rect& rectSRC, const Rect& rectDST, uint32_t level) override
	{
		AKA_ASSERT(dst->format() == this->format(), "Invalid format");
		AKA_ASSERT(rectDST.x + rectDST.w <= dst->width() && rectDST.y + rectDST.h <= dst->height(), "Rect not in range.");
		AKA_ASSERT(rectSRC.x + rectSRC.w <= this->width() && rectSRC.y + rectSRC.h <= this->height(), "Rect not in range.");
		AKA_ASSERT(rectSRC.w == rectDST.w && rectSRC.h == rectDST.h, "Rect size invalid.");
		GLuint copyFBO = 0;
		glGenFramebuffers(1, &copyFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, copyFBO);
		GLenum attachment = GL_COLOR_ATTACHMENT0;
		if (isStencil(m_format))
			attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		else if (isDepth(m_format))
			attachment = GL_DEPTH_ATTACHMENT;
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, m_textureID, level);
		glBindTexture(GL_TEXTURE_2D, (GLuint)dst->handle().value());
		glCopyTexSubImage2D(GL_TEXTURE_2D, level, rectDST.x, rectDST.y, rectSRC.x, rectSRC.y, rectSRC.w, rectSRC.h);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &copyFBO);
		if ((TextureFlag::GenerateMips & dst->flags()) == TextureFlag::GenerateMips)
			dst->generateMips();
	}
	void blit(const Texture2D::Ptr& dst, const Rect& rectSRC, const Rect& rectDST, TextureFilter filter, uint32_t level) override
	{
		AKA_ASSERT(dst->format() == this->format(), "Invalid format");
		AKA_ASSERT(rectSRC.x + rectSRC.w <= this->width() || rectSRC.y + rectSRC.h <= this->height(), "Rect not in range");
		AKA_ASSERT(rectDST.x + rectDST.w <= dst->width() || rectDST.y + rectDST.h <= dst->height(), "Rect not in range");
		GLuint blitFBO[2] = { 0 };
		GLenum attachment = GL_COLOR_ATTACHMENT0;
		GLenum mask = GL_COLOR_BUFFER_BIT;
		if (isDepth(m_format))
		{
			attachment = GL_DEPTH_ATTACHMENT;
			mask = GL_DEPTH_BUFFER_BIT;
		}
		else if (isStencil(m_format))
		{
			attachment = GL_DEPTH_STENCIL_ATTACHMENT;
			mask = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
		}
		glGenFramebuffers(2, blitFBO);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, blitFBO[0]); // src
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, attachment, GL_TEXTURE_2D, m_textureID, level);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blitFBO[1]); // dst
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D, (GLuint)dst->handle().value(), level);
		glBlitFramebuffer(
			rectSRC.x, rectSRC.y, rectSRC.w, rectSRC.h,
			rectDST.x, rectDST.y, rectDST.w, rectDST.h,
			mask,
			gl::filter(filter)
		);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(2, blitFBO);
		if ((TextureFlag::GenerateMips & dst->flags()) == TextureFlag::GenerateMips)
			dst->generateMips();
	}
	TextureHandle handle() const override
	{
		return TextureHandle(static_cast<uintptr_t>(m_textureID));
	}
	void generateMips() override
	{
		if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
			return;
		glGenerateMipmap(GL_TEXTURE_2D);
		m_flags = m_flags | TextureFlag::GenerateMips;
	}
private:
	GLuint m_textureID;
};

class GLTextureCubeMap : public TextureCubeMap
{
public:
	GLTextureCubeMap(
		uint32_t width, uint32_t height, 
		TextureFormat format, TextureFlag flags,
		const void* px = nullptr, const void* nx = nullptr,
		const void* py = nullptr, const void* ny = nullptr,
		const void* pz = nullptr, const void* nz = nullptr
	) :
		TextureCubeMap(width, height, format, flags),
		m_textureID(0)
	{
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, gl::componentInternal(m_format), width, height, 0, gl::component(m_format), gl::format(m_format), px);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, gl::componentInternal(m_format), width, height, 0, gl::component(m_format), gl::format(m_format), nx);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, gl::componentInternal(m_format), width, height, 0, gl::component(m_format), gl::format(m_format), py);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, gl::componentInternal(m_format), width, height, 0, gl::component(m_format), gl::format(m_format), ny);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, gl::componentInternal(m_format), width, height, 0, gl::component(m_format), gl::format(m_format), pz);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, gl::componentInternal(m_format), width, height, 0, gl::component(m_format), gl::format(m_format), nz);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.f);
		if ((TextureFlag::GenerateMips & flags) == TextureFlag::GenerateMips)
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	~GLTextureCubeMap()
	{
		if (m_textureID != 0)
			glDeleteTextures(1, &m_textureID);
	}
	void upload(const void* data, uint32_t layer, uint32_t level) override
	{
		upload(Rect{ 0, 0, m_width, m_height }, data, layer, level);
	}
	void upload(const Rect& rect, const void* data, uint32_t layer, uint32_t level) override
	{
		AKA_ASSERT(rect.x + rect.w <= m_width, "");
		AKA_ASSERT(rect.y + rect.h <= m_height, "");
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, level, (GLint)rect.x, (GLint)rect.y, (GLsizei)rect.w, (GLsizei)rect.h, gl::component(m_format), gl::format(m_format), data);
		if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
			glGenerateMipmap(gl::type(m_type));
	}
	void download(void* data, uint32_t layer, uint32_t level) override
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, level, gl::component(m_format), gl::format(m_format), data);
	}
	TextureHandle handle() const override
	{
		return TextureHandle(static_cast<uintptr_t>(m_textureID));
	}
	void generateMips() override
	{
		if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
			return;
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		m_flags = m_flags | TextureFlag::GenerateMips;
	}
private:
	GLuint m_textureID;
};


class GLTexture2DMultisample : public Texture2DMultisample
{
public:
	GLTexture2DMultisample(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		uint8_t samples,
		const void* data = nullptr
	) :
		Texture2DMultisample(width, height, format, flags),
		m_textureID(0)
	{
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_textureID);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, gl::componentInternal(m_format), width, height, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.f);
		if ((TextureFlag::GenerateMips & flags) == TextureFlag::GenerateMips)
			glGenerateMipmap(GL_TEXTURE_2D_MULTISAMPLE);
	}
	~GLTexture2DMultisample()
	{
		if (m_textureID != 0)
			glDeleteTextures(1, &m_textureID);
	}
	void upload(const void* data) override
	{
		upload(Rect{ 0, 0, m_width, m_height }, data);
	}
	void upload(const Rect& rect, const void* data) override
	{
		AKA_ASSERT(rect.x + rect.w <= m_width, "");
		AKA_ASSERT(rect.y + rect.h <= m_height, "");
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_textureID);
		glTexSubImage2D(GL_TEXTURE_2D_MULTISAMPLE, 0, (GLint)rect.x, (GLint)rect.y, (GLsizei)rect.w, (GLsizei)rect.h, gl::component(m_format), gl::format(m_format), data);
		if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
			glGenerateMipmap(gl::type(m_type));
	}
	void download(void* data) override
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_textureID);
		glGetTexImage(GL_TEXTURE_2D_MULTISAMPLE, 0, gl::component(m_format), gl::format(m_format), data);
	}
	TextureHandle handle() const override
	{
		return TextureHandle(static_cast<uintptr_t>(m_textureID));
	}
	void generateMips() override
	{
		if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
			return;
		glGenerateMipmap(GL_TEXTURE_2D_MULTISAMPLE);
		m_flags = m_flags | TextureFlag::GenerateMips;
	}
private:
	GLuint m_textureID;
};

/*void copy(Texture::Ptr src, Texture::Ptr dst, const Rect& rect)
{
	GLuint m_copyFBO;
	AKA_ASSERT(src->format() == this->format(), "Invalid format");
	AKA_ASSERT(rect.x + rect.w < src->width() || rect.y + rect.h < src->height(), "Rect not in range");
	AKA_ASSERT(rect.x + rect.w < this->width() || rect.y + rect.h < this->height(), "Rect not in range");
	if (m_copyFBO == 0)
		glGenFramebuffers(1, &m_copyFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_copyFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gl::type(m_type), reinterpret_cast<GLTexture*>(src.get())->m_textureID, 0);
	glBindTexture(gl::type(m_type), m_textureID);
	glCopyTexImage2D(gl::type(m_type), 0, gl::componentInternal(m_format), rect.x, rect.y, rect.w, rect.h, 0);
	// TODO copy all mip map level
	glBindTexture(gl::type(m_type), 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &m_copyFBO);
}*/

class GLShader : public Shader
{
public:
	GLShader(ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, ShaderHandle compute, const VertexAttribute* attributes, size_t count) :
		Shader(attributes, count)
	{
		GLuint vert = static_cast<GLuint>(vertex.value());
		GLuint frag = static_cast<GLuint>(fragment.value());
		GLuint comp = static_cast<GLuint>(compute.value());
		GLuint geo = static_cast<GLuint>(geometry.value());
		m_programID = glCreateProgram();
		// Attach shaders
		if (vert != 0 && glIsShader(vert) == GL_TRUE)
			glAttachShader(m_programID, vert);
		if (frag != 0 && glIsShader(frag) == GL_TRUE)
			glAttachShader(m_programID, frag);
		if (comp != 0 && glIsShader(comp) == GL_TRUE)
			glAttachShader(m_programID, comp);
		if (geo != 0 && glIsShader(geo) == GL_TRUE)
			glAttachShader(m_programID, geo);

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
			
			Logger::error("[GL] ", str);
		}
		else
		{
			// Always detach shaders after a successful link.
			if (vert != 0)
			{
				glDetachShader(m_programID, vert);
			}
			if (frag != 0)
			{
				glDetachShader(m_programID, frag);
			}
			if (comp != 0)
			{
				glDetachShader(m_programID, comp);
			}
			if (geo != 0)
			{
				glDetachShader(m_programID, geo);
			}
			glValidateProgram(m_programID);
			GLint status = 0;
			glGetProgramiv(m_programID, GL_VALIDATE_STATUS, &status);
			if (status != GL_TRUE)
				Logger::error("[GL] Program is not valid");
			else
			{
				uint32_t textureUnit = 0;
				GLint activeUniforms = 0;
				glGetProgramiv(m_programID, GL_ACTIVE_UNIFORMS, &activeUniforms);
				// Uniforms
				for (GLint iUniform = 0; iUniform < activeUniforms; iUniform++)
				{
					GLsizei length;
					GLsizei size;
					GLenum type;
					GLchar name[257];
					glGetActiveUniform(m_programID, iUniform, 256, &length, &size, &type, name);
					name[length] = '\0';

					Uniform uniform;
					if (size > 1) // Remove [0] for compat with D3D11 (and simplicity)
						name[String::length(name) - 3] = '\0';
					uniform.name = name;
					uniform.count = size;
					switch (type)
					{
					case GL_IMAGE_2D:
						uniform.type = UniformType::Image2D;
						uniform.shaderType = ShaderType::Compute;
						uniform.binding = textureUnit++;
						break;
					case GL_SAMPLER_2D:
						uniform.type = UniformType::Texture2D;
						uniform.shaderType = ShaderType::Fragment;
						uniform.binding = textureUnit++;
						// TODO add sampler
						break;
					case GL_SAMPLER_CUBE:
						uniform.type = UniformType::TextureCubemap;
						uniform.shaderType = ShaderType::Fragment;
						uniform.binding = textureUnit++;
						break;
					case GL_SAMPLER_2D_MULTISAMPLE:
						uniform.type = UniformType::Texture2DMultisample;
						uniform.shaderType = ShaderType::Fragment;
						uniform.binding = textureUnit++;
						break;
					/*case GL_FLOAT:
						uniform.type = UniformType::Float;
						uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
						break;
					case GL_INT:
						uniform.type = UniformType::Int;
						uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
						break;
					case GL_UNSIGNED_INT:
						uniform.type = UniformType::UnsignedInt;
						uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
						break;
					case GL_FLOAT_VEC2:
						uniform.type = UniformType::Vec2;
						uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
						break;
					case GL_FLOAT_VEC3:
						uniform.type = UniformType::Vec3;
						uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
						break;
					case GL_FLOAT_VEC4:
						uniform.type = UniformType::Vec4;
						uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
						break;
					case GL_FLOAT_MAT3:
						uniform.type = UniformType::Mat3;
						uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
						break;
					case GL_FLOAT_MAT4:
						uniform.type = UniformType::Mat4;
						uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
						break;*/
					default:
						continue;
						//Logger::warn("[GL] Unsupported Uniform Type : ", name);
						//break;
					}
					m_uniforms.push_back(uniform);
				}
				// UBO
				GLint activeUniformBlocks = 0;
				glGetProgramiv(m_programID, GL_ACTIVE_UNIFORM_BLOCKS, &activeUniformBlocks);
				for (GLint iUniform = 0; iUniform < activeUniformBlocks; iUniform++)
				{
					GLsizei length;
					GLchar name[257];
					glGetActiveUniformBlockName(m_programID, iUniform, 257, &length, name);
					name[length] = '\0';

					Uniform uniform;
					uniform.name = name;
					uniform.count = 1;
					uniform.binding = iUniform;
					uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);
					uniform.type = UniformType::Buffer;
					m_uniforms.push_back(uniform);
				};
			}
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
		// TODO This is API agnostic, move in ShaderMaterial.cpp
		GLint activeUniforms = 0;
		uint32_t textureCount = 0;
		uint32_t imageCount = 0;
		uint32_t bufferCount = 0;
		for (const Uniform& uniform : *m_shader)
		{
			switch (uniform.type)
			{
			case UniformType::Buffer:
				bufferCount = max(bufferCount, uniform.binding + uniform.count);
				break;
			case UniformType::TextureCubemap:
			case UniformType::Texture2D:
			case UniformType::Texture2DMultisample:
				textureCount = max(textureCount, uniform.binding + uniform.count);
				break;
			case UniformType::Float:
			case UniformType::Int:
			case UniformType::UnsignedInt:
			case UniformType::Vec2:
			case UniformType::Vec3:
			case UniformType::Vec4:
			case UniformType::Mat3:
			case UniformType::Mat4:
				break;
			default:
				Logger::warn("[GL] Unsupported Uniform Type : ", uniform.name);
				break;
			}
		}
		m_buffers.resize(bufferCount, nullptr);
		m_textures.resize(textureCount, nullptr);
		m_samplers.resize(textureCount, TextureSampler::nearest);
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
		glUseProgram(glShader->getProgramID());
		for (const Uniform& uniform : *m_shader)
		{
			// TODO map this for performance
			GLuint programID = reinterpret_cast<GLShader*>(m_shader.get())->getProgramID();
			GLint location = glGetUniformLocation(programID, uniform.name.cstr());
			switch (uniform.type)
			{
			default:
			case UniformType::None:
				Logger::error("[GL] Unsupported uniform type : ", uniform.name, "(", (int)uniform.type, ")");
				break;
			case UniformType::Mat4:
			case UniformType::Mat3:
			case UniformType::Float:
			case UniformType::Int:
			case UniformType::UnsignedInt:
			case UniformType::Vec2:
			case UniformType::Vec3:
			case UniformType::Vec4:
				break;
			case UniformType::Buffer: {
				for (uint32_t i = 0; i < uniform.count; i++)
				{
					Buffer::Ptr buffer = m_buffers[uniform.binding + i];
					GLuint blockID = glGetUniformBlockIndex(programID, uniform.name.cstr());
					glUniformBlockBinding(programID, blockID, uniform.binding + i);
					glBindBufferBase(GL_UNIFORM_BUFFER, uniform.binding + i, (GLuint)buffer->handle().value());
					//glBindBufferRange
				}
				break;
			}
			case UniformType::Texture2D:
			case UniformType::Texture2DMultisample:
			case UniformType::TextureCubemap: {
				std::vector<GLint> units;
				// Bind texture to units.
				for (uint32_t i = 0; i < uniform.count; i++)
				{
					Texture::Ptr texture = m_textures[uniform.binding + i];
					TextureSampler sampler = m_samplers[uniform.binding + i];
					glActiveTexture(GL_TEXTURE0 + uniform.binding + i);
					if (texture != nullptr)
					{
						// Bind and set sampler
						GLenum type = gl::type(texture->type());
						glBindTexture(type, (GLuint)texture->handle().value());
						glTexParameteri(type, GL_TEXTURE_MAG_FILTER, gl::filter(sampler.filterMag, TextureMipMapMode::None));
						glTexParameteri(type, GL_TEXTURE_MIN_FILTER, gl::filter(sampler.filterMin, sampler.mipmapMode));
						glTexParameteri(type, GL_TEXTURE_WRAP_S, gl::wrap(sampler.wrapU));
						glTexParameteri(type, GL_TEXTURE_WRAP_T, gl::wrap(sampler.wrapV));
						glTexParameteri(type, GL_TEXTURE_WRAP_R, gl::wrap(sampler.wrapW));
						glTexParameterf(type, GL_TEXTURE_MAX_ANISOTROPY_EXT, sampler.anisotropy); // GL_MAX_TEXTURE_MAX_ANISOTROPY
						if ((sampler.mipmapMode != TextureMipMapMode::None) && ((texture->flags() & TextureFlag::GenerateMips) != TextureFlag::GenerateMips))
							texture->generateMips();
					}
					else
					{
						GLenum glType;
						switch (uniform.type)
						{
						default:
						case UniformType::Texture2D:
							glType = GL_TEXTURE_2D;
							break;
						case UniformType::Texture2DMultisample:
							glType = GL_TEXTURE_2D_MULTISAMPLE;
							break;
						case UniformType::TextureCubemap:
							glType = GL_TEXTURE_CUBE_MAP;
							break;
						}
						glBindTexture(glType, 0);
					}
					units.push_back(uniform.binding + i);
				}
				// Upload texture unit array.
				glUniform1iv(location, (GLsizei)units.size(), units.data());
				break;
			}
			/*case UniformType::Image2D: {
				std::vector<GLint> units;
				// Bind images to units.
				for (uint32_t i = 0; i < uniform.count; i++)
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
				glUniform1iv(location, (GLsizei)units.size(), units.data());
				break;
			}*/
			}
		}
		//GLint maxTextureUnits = -1;
		//glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
		//AKA_ASSERT(textureUnit <= maxTextureUnits, "Cannot handle so many textures in a single pass.");
	}
};

class GLBuffer : public Buffer
{
	static GLenum type(BufferType type) {
		switch (type) {
		case BufferType::Vertex:
			return GL_ARRAY_BUFFER;
		case BufferType::Index:
			return GL_ELEMENT_ARRAY_BUFFER;
		case BufferType::Uniform:
			return GL_UNIFORM_BUFFER;
		case BufferType::ShaderStorage:
			return GL_SHADER_STORAGE_BUFFER;
		default:
			return 0;
		}
	}
	static GLenum access(BufferUsage usage, BufferCPUAccess access) {
		switch (usage) {
		case BufferUsage::Default:
			return GL_STATIC_DRAW;
		case BufferUsage::Immutable:
			return GL_STATIC_DRAW;
		case BufferUsage::Staging:
			return (access == BufferCPUAccess::Read ? GL_STREAM_READ : (access == BufferCPUAccess::Write ? GL_STREAM_DRAW : GL_STREAM_COPY));
		case BufferUsage::Dynamic:
			AKA_ASSERT(access == BufferCPUAccess::Write || access == BufferCPUAccess::None, "Invalid access");
			return GL_DYNAMIC_DRAW;
		default:
			return 0;
		}
	}
public:
	GLBuffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data) :
		Buffer(type, size, usage, access),
		m_glType(GLBuffer::type(type)),
		m_bufferID(0)
	{
		glGenBuffers(1, &m_bufferID);

		glBindBuffer(m_glType, m_bufferID);
		glBufferData(m_glType, size, data, GLBuffer::access(usage, access));
		glBindBuffer(m_glType, 0);
	}
	GLBuffer(const GLBuffer&) = delete;
	GLBuffer& operator=(const GLBuffer&) = delete;
	~GLBuffer()
	{
		if (m_bufferID)
			glDeleteBuffers(1, &m_bufferID);
	}

	void reallocate(size_t size, const void* data) override
	{
		glBindBuffer(m_glType, m_bufferID);
		glBufferData(m_glType, size, data, GLBuffer::access(m_usage, m_access));
		glBindBuffer(m_glType, 0);
	}

	void upload(const void* data, size_t size, size_t offset = 0) override
	{
		AKA_ASSERT(m_usage != BufferUsage::Immutable, "Cannot upload to static buffer.");
		glBindBuffer(m_glType, m_bufferID);
		glBufferSubData(m_glType, offset, size, data);
		glBindBuffer(m_glType, 0);
	}

	void upload(const void* data) override
	{
		AKA_ASSERT(m_usage != BufferUsage::Immutable, "Cannot upload to static buffer.");
		glBindBuffer(m_glType, m_bufferID);
		glBufferSubData(m_glType, 0, m_size, data);
		glBindBuffer(m_glType, 0);
	}

	void download(void* data, size_t size, size_t offset = 0) override
	{
		glBindBuffer(m_glType, m_bufferID);
		glGetBufferSubData(m_glType, offset, size, data);
		glBindBuffer(m_glType, 0);
	}

	void download(void* data) override
	{
		glBindBuffer(m_glType, m_bufferID);
		glGetBufferSubData(m_glType, 0, m_size, data);
		glBindBuffer(m_glType, 0);
	}

	void* map(BufferMap access) override
	{
		glBindBuffer(m_glType, m_bufferID);
		GLenum glAccess = GL_READ_ONLY;
		switch (access)
		{
		case BufferMap::Read:
			AKA_ASSERT(m_access == BufferCPUAccess::Read || m_access == BufferCPUAccess::ReadWrite, "");
			glAccess = GL_READ_ONLY;
			break;
		case BufferMap::WriteDiscard:
		case BufferMap::WriteNoOverwrite:
		case BufferMap::Write:
			AKA_ASSERT(m_access == BufferCPUAccess::Write || m_access == BufferCPUAccess::ReadWrite, "");
			glAccess = GL_WRITE_ONLY;
			break;
		case BufferMap::ReadWrite:
			AKA_ASSERT(m_access == BufferCPUAccess::ReadWrite, "");
			glAccess = GL_READ_WRITE;
			break;
		default:
			Logger::error("Buffer map type not supported");
			break;
		}
		void* data = glMapBuffer(m_glType, glAccess);
		glBindBuffer(m_glType, 0);
		return data;
	}

	void unmap() override
	{
		glBindBuffer(m_glType, m_bufferID);
		glUnmapBuffer(m_glType);
		glBindBuffer(m_glType, 0);
	}

	BufferHandle handle() const override { return BufferHandle(m_bufferID); }
private:
	GLenum m_glType;
	GLuint m_bufferID;
};

class GLMesh : public Mesh
{
public:
	GLMesh() :
		Mesh(),
		m_vao(0)
	{
		glGenVertexArrays(1, &m_vao);
	}
	GLMesh(const GLMesh&) = delete;
	GLMesh& operator=(const GLMesh&) = delete;
	~GLMesh()
	{
		if (m_vao)
			glDeleteVertexArrays(1, &m_vao);
	}
public:
	void upload(const VertexAccessor* vertexAccessor, size_t accessorCount, const IndexAccessor& indexAccessor) override
	{
		// --- Vertices
		glBindVertexArray(m_vao);
		// Setup correct channels
		for (size_t i = m_vertexAccessors.size(); i < accessorCount; i++)
			glEnableVertexAttribArray((GLuint)i);
		for (size_t i = accessorCount; i < m_vertexAccessors.size(); i++)
			glDisableVertexAttribArray((GLuint)i);
		for (size_t i = 0; i < accessorCount; i++)
		{
			const VertexAccessor& a = vertexAccessor[i];
			//GLint componentSize = size(a.attribute.format);
			GLint componentCount = size(a.attribute.type);
			GLenum componentType = gl::format(a.attribute.format);
			GLboolean normalized = GL_FALSE;
			GLsizei stride = a.bufferView.stride;
			size_t offset = a.bufferView.offset + a.offset;
			glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(a.bufferView.buffer->handle().value()));
			glVertexAttribPointer((GLuint)i, componentCount, componentType, normalized, stride, (void*)(uintptr_t)offset);
		}
		m_vertexAccessors.clear();
		m_vertexAccessors.insert(m_vertexAccessors.end(), vertexAccessor, vertexAccessor + accessorCount);

		// --- Indices
		if (indexAccessor.bufferView.buffer != nullptr)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(indexAccessor.bufferView.buffer->handle().value()));
			m_indexAccessor = indexAccessor;
		}
		else
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			m_indexAccessor = {};
		}
		glBindVertexArray(0);
	}

	void upload(const VertexAccessor* vertexAccessor, size_t accessorCount) override
	{
		upload(vertexAccessor, accessorCount, IndexAccessor{ IndexBufferView{ nullptr, 0, 0 }, IndexFormat::UnsignedByte });
	}

	void draw(PrimitiveType type, uint32_t vertexCount, uint32_t vertexOffset) const override
	{
		glBindVertexArray(m_vao);
		glDrawArrays(gl::primitive(type), vertexOffset, vertexCount);
		glBindVertexArray(0);
	}
	void drawIndexed(PrimitiveType type, uint32_t indexCount, uint32_t indexOffset) const override
	{
		AKA_ASSERT(m_indexAccessor.bufferView.buffer != nullptr, "Need indices to call drawIndexed");
		glBindVertexArray(m_vao);
		glDrawElements(
			gl::primitive(type),
			static_cast<GLsizei>(indexCount),
			gl::format(m_indexAccessor.format),
			(void*)((uintptr_t)size(m_indexAccessor.format) * (uintptr_t)indexOffset)
		);
		glBindVertexArray(0);
	}

private:
	GLuint m_vao;
};

class GLFramebuffer : public Framebuffer
{
public:
	GLFramebuffer(Attachment* attachments, size_t count) :
		Framebuffer(attachments, count),
		m_framebufferID(0)
	{
		glGenFramebuffers(1, &m_framebufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
		std::vector<GLenum> drawBuffers;
		for (size_t iAtt = 0; iAtt < count; iAtt++)
		{
			attach(attachments[iAtt]);
			GLenum attachmentType = gl::attachmentType(attachments[iAtt].type);
			if (attachmentType >= GL_COLOR_ATTACHMENT0 && attachmentType <= GL_COLOR_ATTACHMENT15)
				drawBuffers.push_back(attachmentType);
		}
		AKA_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not created");
		glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
	}
	~GLFramebuffer()
	{
		if (m_framebufferID != 0)
			glDeleteFramebuffers(1, &m_framebufferID);
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
	void blit(Framebuffer::Ptr src, Rect rectSrc, Rect rectDst, AttachmentType type, TextureFilter filter) override
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebufferID);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, reinterpret_cast<GLFramebuffer*>(src.get())->m_framebufferID);
		GLenum mask = 0;
		switch (type)
		{
		case AttachmentType::Color0:
		case AttachmentType::Color1:
		case AttachmentType::Color2:
		case AttachmentType::Color3:
			mask |= GL_COLOR_BUFFER_BIT;
			break;
		case AttachmentType::Depth:
			mask |= GL_DEPTH_BUFFER_BIT;
			break;
		case AttachmentType::Stencil:
			mask |= GL_STENCIL_BUFFER_BIT;
			break;
		case AttachmentType::DepthStencil:
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
			// Set correct draw buffers to support new attachment
			std::vector<GLenum> drawBuffers;
			for (Attachment& attachment : m_attachments)
			{
				GLenum attachmentType = gl::attachmentType(attachment.type);
				if (attachmentType >= GL_COLOR_ATTACHMENT0 && attachmentType <= GL_COLOR_ATTACHMENT15)
					drawBuffers.push_back(attachmentType);
			}
			glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
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
		// Attach new attachment to framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
		attach(*attachment);
		AKA_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not valid");

		// Recompute size
		computeSize();
	}
	void attach(const Attachment& attachment)
	{
		GLenum attachmentType = gl::attachmentType(attachment.type);
		GLuint textureID = (GLuint)attachment.texture->handle().value();
		switch (attachment.texture->type())
		{
		case TextureType::Texture1D:
			glFramebufferTexture1D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_1D, textureID, attachment.level);
			break;
		case TextureType::Texture2D:
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, textureID, attachment.level);
			break;
		case TextureType::Texture3D:
			glFramebufferTexture3D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_3D, textureID, attachment.level, attachment.layer);
			break;
		case TextureType::TextureCubeMap:
			if ((AttachmentFlag::AttachTextureObject & attachment.flag) == AttachmentFlag::AttachTextureObject)
				glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, textureID, attachment.level);
			else
				glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_CUBE_MAP_POSITIVE_X + attachment.layer, textureID, attachment.level);
			break;
		case TextureType::Texture1DArray:
		case TextureType::Texture2DArray:
			if ((AttachmentFlag::AttachTextureObject & attachment.flag) == AttachmentFlag::AttachTextureObject)
				glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, textureID, attachment.level);
			else
				glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentType, textureID, attachment.level, attachment.layer);
			break;
		case TextureType::TextureCubeMapArray:
			if ((AttachmentFlag::AttachTextureObject & attachment.flag) == AttachmentFlag::AttachTextureObject)
				glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentType, textureID, attachment.level, attachment.layer);
			else
			{
				// TODO allow to attach single face ?
			}
			break;
		}
	}
	void computeSize()
	{
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
	void onReceive(const BackbufferResizeEvent& event) override
	{
		m_width = event.width;
		m_height = event.height;
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
	void blit(Framebuffer::Ptr src, Rect rectSrc, Rect rectDst, AttachmentType type, TextureFilter filter) override
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, reinterpret_cast<GLFramebuffer*>(src.get())->getFramebufferID());
		GLenum mask = 0;
		switch (type)
		{
		case AttachmentType::Color0:
		case AttachmentType::Color1:
		case AttachmentType::Color2:
		case AttachmentType::Color3:
			mask |= GL_COLOR_BUFFER_BIT;
			break;
		case AttachmentType::Depth:
			mask |= GL_DEPTH_BUFFER_BIT;
			break;
		case AttachmentType::Stencil:
			mask |= GL_STENCIL_BUFFER_BIT;
			break;
		case AttachmentType::DepthStencil:
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
	void set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag, uint32_t layer, uint32_t level) override
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
			GLenum frontFunc = gl::stencilCompare(pass.stencil.front.mode);
			GLenum backFunc = gl::stencilCompare(pass.stencil.back.mode);
			glStencilFuncSeparate(GL_FRONT, frontFunc, 1, pass.stencil.readMask);
			glStencilFuncSeparate(GL_BACK, backFunc, 1, pass.stencil.readMask);
			glStencilOpSeparate(
				GL_FRONT,
				gl::stencilMode(pass.stencil.front.stencilFailed),
				gl::stencilMode(pass.stencil.front.stencilDepthFailed),
				gl::stencilMode(pass.stencil.front.stencilPassed)
			);
			glStencilOpSeparate(
				GL_BACK,
				gl::stencilMode(pass.stencil.back.stencilFailed),
				gl::stencilMode(pass.stencil.back.stencilDepthFailed),
				gl::stencilMode(pass.stencil.back.stencilPassed)
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
		if (pass.submesh.mesh->isIndexed())
			pass.submesh.drawIndexed();
		else
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
	Image image(gctx.backbuffer->width(), gctx.backbuffer->height(), 4, ImageFormat::UnsignedByte);
	glReadPixels(0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data());
	image.encodePNG(path);
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

Texture2D::Ptr GraphicBackend::createTexture2D(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	const void* data
)
{
	return std::make_shared<GLTexture2D>(width, height, format, flags, data);
}

Texture2DMultisample::Ptr GraphicBackend::createTexture2DMultisampled(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	uint8_t samples,
	const void* data
)
{
	return std::make_shared<GLTexture2DMultisample>(width, height, format, flags, samples, data);
}

TextureCubeMap::Ptr GraphicBackend::createTextureCubeMap(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	const void* px, const void* nx,
	const void* py, const void* ny,
	const void* pz, const void* nz
)
{
	return std::make_shared<GLTextureCubeMap>(width, height, format, flags, px, nx, py, ny, pz, nz);
}

Framebuffer::Ptr GraphicBackend::createFramebuffer(Attachment* attachments, size_t count)
{
	return std::make_shared<GLFramebuffer>(attachments, count);
}

Buffer::Ptr GraphicBackend::createBuffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return std::make_shared<GLBuffer>(type, size, usage, access, data);
}

Mesh::Ptr GraphicBackend::createMesh()
{
	return std::make_shared<GLMesh>();
}

ShaderHandle GraphicBackend::compile(const char* content, ShaderType type)
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
		return ShaderHandle(0);
	}
	return ShaderHandle(shaderID);
}

void GraphicBackend::destroy(ShaderHandle handle)
{
	glDeleteShader((GLuint)handle.value());
}

Shader::Ptr GraphicBackend::createShader(ShaderHandle vert, ShaderHandle frag, const VertexAttribute* attributes, size_t count)
{
	return std::make_shared<GLShader>(vert, frag, ShaderHandle(0), ShaderHandle(0), attributes, count);
}

Shader::Ptr GraphicBackend::createShaderGeometry(ShaderHandle vert, ShaderHandle frag, ShaderHandle geometry, const VertexAttribute* attributes, size_t count)
{
	return std::make_shared<GLShader>(vert, frag, geometry, ShaderHandle(0), attributes, count);
}

Shader::Ptr GraphicBackend::createShaderCompute(ShaderHandle compute)
{
	VertexAttribute dummy{};
	return std::make_shared<GLShader>(ShaderHandle(0), ShaderHandle(0), ShaderHandle(0), compute, &dummy, 0);
}

ShaderMaterial::Ptr aka::GraphicBackend::createShaderMaterial(Shader::Ptr shader)
{
	return std::make_shared<GLShaderMaterial>(shader);
}

};
#endif
