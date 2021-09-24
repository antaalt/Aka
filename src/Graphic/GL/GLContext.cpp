#if defined(AKA_USE_OPENGL)

#include "GLContext.h"

#if defined(AKA_PLATFORM_WINDOWS)
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace aka {

const char* glGetErrorString(GLenum error)
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

GLuint glType(ShaderType type)
{
	switch (type)
	{
	case ShaderType::Vertex:
		return GL_VERTEX_SHADER;
		//case ShaderType::TessControl:
		//	return GL_TESS_CONTROL_SHADER;
		//case ShaderType::TessEvaluation:
		//	return GL_TESS_EVALUATION_SHADER;
	case ShaderType::Fragment:
		return GL_FRAGMENT_SHADER;
	case ShaderType::Geometry:
		return GL_GEOMETRY_SHADER;
	case ShaderType::Compute:
		return GL_COMPUTE_SHADER;
	default:
		throw std::runtime_error("Not implemented.");
	}
}

GLenum glType(BufferType type) {
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
		throw std::runtime_error("Not implemented.");
	}
}

GLenum glAccess(BufferUsage usage, BufferCPUAccess access) {
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
		throw std::runtime_error("Not implemented.");
	}
}

GLenum glFilter(TextureFilter type, TextureMipMapMode mode)
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
	throw std::runtime_error("Not implemented.");
}

GLenum glAttachmentType(AttachmentType type)
{
	switch (type)
	{
	default:
		throw std::runtime_error("Not implemented.");
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

GLenum glFormat(IndexFormat format) {
	switch (format) {
	default:
		throw std::runtime_error("Not implemented.");
	case IndexFormat::UnsignedByte:
		return GL_UNSIGNED_BYTE;
	case IndexFormat::UnsignedShort:
		return GL_UNSIGNED_SHORT;
	case IndexFormat::UnsignedInt:
		return GL_UNSIGNED_INT;
	}
}

GLenum glFormat(VertexFormat format) {
	switch (format) {
	default:
		throw std::runtime_error("Not implemented.");
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

GLenum glPrimitive(PrimitiveType type)
{
	switch (type)
	{
	default:
		throw std::runtime_error("Not implemented.");
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
	case PrimitiveType::Triangles:
		return GL_TRIANGLES;
	}
}

GLenum glWrap(TextureWrap wrap)
{
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

GLenum glComponentInternal(TextureFormat format) {
	switch (format) {
	default: throw std::runtime_error("Not implemented");
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

GLenum glComponent(TextureFormat component) {
	switch (component) {
	default:
		throw std::runtime_error("Not implemented");
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

GLenum glType(TextureType type) {
	switch (type) {
	default: throw std::runtime_error("Not implemented");
	case TextureType::Texture2D: return GL_TEXTURE_2D;
	case TextureType::TextureCubeMap: return GL_TEXTURE_CUBE_MAP;
	case TextureType::Texture2DMultisample: return GL_TEXTURE_2D_MULTISAMPLE;
	}
}

GLenum glFormat(TextureFormat format) {
	switch (format) {
	default: throw std::runtime_error("Not implemented");
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

GLenum glBlendMode(BlendMode mode)
{
	switch (mode)
	{
	default: throw std::runtime_error("Not implemented");
	case BlendMode::Zero:
		return GL_ZERO;
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

GLenum glBlendOp(BlendOp op)
{
	switch (op)
	{
	default: throw std::runtime_error("Not implemented");
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

GLenum glStencilCompare(StencilCompare mode)
{
	switch (mode)
	{
	default: throw std::runtime_error("Not implemented");
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

GLenum glStencilMode(StencilMode op)
{
	switch (op)
	{
	default: throw std::runtime_error("Not implemented");
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

};

#endif
