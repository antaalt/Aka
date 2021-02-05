#if defined(AKA_USE_OPENGL)
#include "../GraphicBackend.h"
#include "../../Core/Debug.h"
#include "../../OS/Logger.h"
#include "../../OS/Image.h"
#include "../../Platform/PlatformBackend.h"

#define GLEW_NO_GLU
#include <gl/glew.h>
#include <gl/gl.h>
#include <GLFW/glfw3.h>

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
			STRINGIFY(result),                 \
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
	std::string errorType;
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		errorType = "error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		errorType = "deprecated_behaviour";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		errorType = "undefined_behaviour";
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
	}
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		aka::Logger::debug("[", errorType, "][low] ", message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		aka::Logger::warn("[", errorType, "][medium] ", message);
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		aka::Logger::error("[", errorType, "][high] ", message);
		break;
	}
}

uint32_t checkError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:
			error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error = "INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			error = "INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			error = "STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			error = "STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			error = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}
		std::cerr << error << " | " << file << " (" << line << ")" << std::endl;
		throw std::runtime_error("Error");
	}
	return errorCode;
}

#define checkError() checkError_(__FILE__, __LINE__)

namespace aka {

namespace gl {
GLenum attachmentType(Framebuffer::AttachmentType type)
{
	switch (type)
	{
	default:
		Logger::warn("Framebuffer attachment type not defined : ", (int)type);
		return GL_COLOR_ATTACHMENT0;
	case Framebuffer::AttachmentType::Color0:
		return GL_COLOR_ATTACHMENT0;
	case Framebuffer::AttachmentType::Color1:
		return GL_COLOR_ATTACHMENT1;
	case Framebuffer::AttachmentType::Color2:
		return GL_COLOR_ATTACHMENT2;
	case Framebuffer::AttachmentType::Color3:
		return GL_COLOR_ATTACHMENT3;
	case Framebuffer::AttachmentType::Depth:
		return GL_DEPTH_ATTACHMENT;
	case Framebuffer::AttachmentType::Stencil:
		return GL_STENCIL_ATTACHMENT;
	}
}

GLenum framebufferType(Framebuffer::Type type) {
	switch (type) {
	case Framebuffer::Type::Read:
		return GL_READ_FRAMEBUFFER;
	case Framebuffer::Type::Draw:
		return GL_DRAW_FRAMEBUFFER;
	default:
	case Framebuffer::Type::Both:
		return GL_FRAMEBUFFER;
	}
}

GLenum filter(Sampler::Filter type) {
	switch (type) {
	default:
	case Sampler::Filter::Linear:
		return GL_LINEAR;
	case Sampler::Filter::Nearest:
		return GL_NEAREST;
	case Sampler::Filter::MipMapNearest:
		return GL_NEAREST_MIPMAP_NEAREST;
	case Sampler::Filter::MipMapLinear:
		return GL_LINEAR_MIPMAP_LINEAR;
	}
}

GLenum wrap(Sampler::Wrap wrap) {
	switch (wrap) {
	default:
	case Sampler::Wrap::Clamp:
		return GL_CLAMP_TO_EDGE;
	case Sampler::Wrap::Repeat:
		return GL_REPEAT;
	case Sampler::Wrap::Mirror:
		return GL_MIRRORED_REPEAT;
	}
}

GLenum format(Texture::Format format) {
	switch (format) {
	default:
		throw std::runtime_error("Not implemneted");
	case Texture::Format::Red:
		return GL_RED;
	case Texture::Format::Rgba:
		return GL_RGBA;
	case Texture::Format::Rgba8:
		return GL_RGBA8;
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
		return 0;
	}
}

GLenum blendMode(BlendMode mode)
{
	switch (mode)
	{
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
	default:
		return 0;
	}
}
GLenum blendOp(BlendOp op)
{
	switch (op)
	{
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
	default:
		return 0;
	}
}

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
			throw std::runtime_error(str);
		}
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

		GLint active_uniforms = 0;
		glGetProgramiv(m_programID, GL_ACTIVE_UNIFORMS, &active_uniforms);
		for (GLint iUniform = 0; iUniform < active_uniforms; iUniform++)
		{
			GLsizei length;
			GLsizei size;
			GLenum type;
			GLchar name[257];
			glGetActiveUniform(m_programID, iUniform, 256, &length, &size, &type, name);
			name[length] = '\0';
			switch (type)
			{
			case GL_SAMPLER_2D: {
				Uniform texUniform;
				texUniform.id = UniformID(glGetUniformLocation(m_programID, name));
				texUniform.name = name;
				texUniform.bufferIndex = 0;
				texUniform.arrayLength = size;
				texUniform.type = UniformType::Texture2D;
				texUniform.shaderType = ShaderType::Fragment;
				m_uniforms.push_back(texUniform);
				// TODO add sampler
				break;
			}
			case GL_FLOAT:
			case GL_FLOAT_VEC2:
			case GL_FLOAT_VEC3:
			case GL_FLOAT_VEC4:
			case GL_FLOAT_MAT3:
			case GL_FLOAT_MAT4: {
				Uniform uniform;
				uniform.id = UniformID(glGetUniformLocation(m_programID, name));
				uniform.name = name;
				uniform.type = UniformType::None;
				uniform.bufferIndex = 0;
				uniform.arrayLength = size;
				uniform.shaderType = (ShaderType)((int)ShaderType::Vertex | (int)ShaderType::Fragment);

				if (type == GL_FLOAT)
					uniform.type = UniformType::Vec;
				else if (type == GL_FLOAT_VEC2)
					uniform.type = UniformType::Vec2;
				else if (type == GL_FLOAT_VEC3)
					uniform.type = UniformType::Vec3;
				else if (type == GL_FLOAT_VEC4)
					uniform.type = UniformType::Vec4;
				else if (type == GL_FLOAT_MAT3)
					uniform.type = UniformType::Mat3;
				else if (type == GL_FLOAT_MAT4)
					uniform.type = UniformType::Mat4;
				else
				{
					Logger::error("Unsupported Uniform Type : ", type);
					break;
				}
				m_uniforms.push_back(uniform);
				break;
			}
			default:
				Logger::warn("Unsupported Uniform Type : ", type);
				break;
			}
		}

		glValidateProgram(m_programID);
	}
	GLShader(const GLShader&) = delete;
	GLShader& operator=(const GLShader&) = delete;
	~GLShader()
	{
		if (m_programID != 0)
			glDeleteProgram(m_programID);
	}
public:
	void use() override
	{
		glUseProgram(m_programID);
	}
	void setFloat1(const char* name, float value) override
	{
		glUseProgram(m_programID);
		glUniform1f((GLint)getUniformID(name).value(), value);
	}
	void setFloat2(const char* name, float x, float y) override
	{
		glUseProgram(m_programID);
		glUniform2f((GLint)getUniformID(name).value(), x, y);
	}
	void setFloat3(const char* name, float x, float y, float z) override
	{
		glUseProgram(m_programID);
		glUniform3f((GLint)getUniformID(name).value(), x, y, z);
	}
	void setFloat4(const char* name, float x, float y, float z, float w) override
	{
		glUseProgram(m_programID);
		glUniform4f((GLint)getUniformID(name).value(), x, y, z, w);
	}
	void setUint1(const char* name, uint32_t value) override
	{
		glUseProgram(m_programID);
		glUniform1ui((GLint)getUniformID(name).value(), value);
	}
	void setUint2(const char* name, uint32_t x, uint32_t y) override
	{
		glUseProgram(m_programID);
		glUniform2ui((GLint)getUniformID(name).value(), x, y);
	}
	void setUint3(const char* name, uint32_t x, uint32_t y, uint32_t z) override
	{
		glUseProgram(m_programID);
		glUniform3ui((GLint)getUniformID(name).value(), x, y, z);
	}
	void setUint4(const char* name, uint32_t x, uint32_t y, uint32_t z, uint32_t w) override
	{
		glUseProgram(m_programID);
		glUniform4ui((GLint)getUniformID(name).value(), x, y, z, w);
	}
	void setInt1(const char* name, int32_t value) override
	{
		glUseProgram(m_programID);
		glUniform1i((GLint)getUniformID(name).value(), value);
	}
	void setInt2(const char* name, int32_t x, int32_t y) override
	{
		glUseProgram(m_programID);
		glUniform2i((GLint)getUniformID(name).value(), x, y);
	}
	void setInt3(const char* name, int32_t x, int32_t y, int32_t z) override
	{
		glUseProgram(m_programID);
		glUniform3i((GLint)getUniformID(name).value(), x, y, z);
	}
	void setInt4(const char* name, int32_t x, int32_t y, int32_t z, int32_t w) override
	{
		glUseProgram(m_programID);
		glUniform4i((GLint)getUniformID(name).value(), x, y, z, w);
	}
	void setMatrix3(const char* name, const float* data, bool transpose) override
	{
		glUseProgram(m_programID);
		glUniformMatrix3fv((GLint)getUniformID(name).value(), 1, transpose, data);
	}
	void setMatrix4(const char* name, const float* data, bool transpose) override
	{
		glUseProgram(m_programID);
		glUniformMatrix4fv((GLint)getUniformID(name).value(), 1, transpose, data);
	}
private:
	GLuint m_programID;
};

class GLTexture : public Texture
{
public:
	GLTexture(uint32_t width, uint32_t height, Format format, const uint8_t* data, Sampler sampler, bool isFramebuffer) :
		Texture(width, height),
		m_format(gl::format(format)),
		m_isFramebuffer(isFramebuffer)
	{
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, m_format, width, height, 0, m_format, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl::filter(sampler.filterMag));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl::filter(sampler.filterMin));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl::wrap(sampler.wrapS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl::wrap(sampler.wrapT));
		if (sampler.filterMin == Sampler::Filter::MipMapLinear || sampler.filterMin == Sampler::Filter::MipMapNearest)
			glGenerateMipmap(GL_TEXTURE_2D);
	}
	GLTexture(GLTexture&) = delete;
	GLTexture& operator=(GLTexture&) = delete;
	~GLTexture()
	{
		if (m_textureID != 0)
			glDeleteTextures(1, &m_textureID);
	}
	void upload(const uint8_t* data) override
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, m_format, m_width, m_height, 0, m_format, GL_UNSIGNED_BYTE, data);
	}
	void upload(const Rect& rect, const uint8_t* data) override
	{
		ASSERT(rect.x + rect.w < m_width, "");
		ASSERT(rect.y + rect.h < m_height, "");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, (GLint)rect.x, (GLint)rect.y, (GLsizei)rect.w, (GLsizei)rect.h, m_format, GL_UNSIGNED_BYTE, data);
	}
	void download(uint8_t* data) override
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glGetTexImage(GL_TEXTURE_2D, 0, m_format, GL_UNSIGNED_BYTE, data);
	}
	Handle handle() override
	{
		return Handle((uintptr_t)m_textureID);
	}
	bool isFramebuffer() override
	{
		return m_isFramebuffer;
	}
	GLuint getTextureID() const
	{ 
		return m_textureID;
	}
private:
	GLenum m_format;
	GLuint m_textureID;
	bool m_isFramebuffer;
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
	void draw(uint32_t indexCount, uint32_t indexOffset) const override
	{
		glBindVertexArray(m_vao);
		// We could use glDrawArrays for non indexed array.
		// But using indexed allow to reduce the number of draw call for different types
		// We can also reuse vertices with glDrawElements as they are indexed
		if (m_indexCount > 0)
		{
			void* indices = (void*)(uintptr_t)(m_indexSize * indexOffset);
			glDrawElements(
				GL_TRIANGLES,
				static_cast<GLsizei>(indexCount),
				gl::format(m_indexFormat),
				indices
			);
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
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
	GLFramebuffer(uint32_t width, uint32_t height, AttachmentType* attachments, size_t count, Sampler sampler) :
		Framebuffer(width, height),
		m_framebufferID(0)
	{
		glGenFramebuffers(1, &m_framebufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
		for (size_t iAtt = 0; iAtt < count; iAtt++)
		{
			Texture::Format format;
			switch (attachments[iAtt])
			{
			case AttachmentType::Color0:
			case AttachmentType::Color1:
			case AttachmentType::Color2:
			case AttachmentType::Color3:
				format = Texture::Format::Rgba;
				break;
			case AttachmentType::Depth:
			case AttachmentType::Stencil:
			case AttachmentType::DepthStencil:
				format = Texture::Format::DepthStencil;
				break;
			}
			std::shared_ptr<GLTexture> tex = std::make_shared<GLTexture>(width, height, format, nullptr, sampler, true);
			glFramebufferTexture2D(GL_FRAMEBUFFER, gl::attachmentType(attachments[iAtt]), GL_TEXTURE_2D, tex->getTextureID(), 0);
			m_attachments.emplace_back();
			Attachment& att = m_attachments.back();
			att.type = attachments[iAtt];
			att.texture = tex;
		}
		ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not created");
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
		throw std::runtime_error("Not implemented");
	}
	void clear(float r, float g, float b, float a) override
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebufferID);
		glClearColor(r, g, b, a);
		glClearDepth(1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	Texture::Ptr attachment(AttachmentType type) override
	{
		for (Attachment& attachment : m_attachments)
		{
			if (attachment.type == type)
				return attachment.texture;
		}
		return nullptr;
	}
	GLuint getFramebufferID() const { return m_framebufferID; }
private:
	std::vector<Attachment> m_attachments;
	GLuint m_framebufferID;
};

class GLBackBuffer : public Framebuffer
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
	void clear(float r, float g, float b, float a) override
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClearColor(r, g, b, a);
		glClearDepth(1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	Texture::Ptr attachment(AttachmentType type) override
	{
		// We do not have access to backbuffer attachment with GL.
		return nullptr;
	}
};

struct GLContext {
	std::shared_ptr<GLBackBuffer> backbuffer = nullptr;
	bool vsync = true;
};

GLContext ctx;

void GraphicBackend::initialize(uint32_t width, uint32_t height)
{
	Device device = getDevice(0);
	Logger::info("Device vendor : ", device.vendor);
	Logger::info("Device renderer : ", device.renderer);
	Logger::info("Device memory : ", device.memory);
	Logger::info("Device version : ", device.version);

#if !defined(__APPLE__)
	glewExperimental = true; // Nécessaire dans le profil de base
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error("Could not init GLEW");
	}
#endif

#if defined(DEBUG)
	if (glDebugMessageCallback) {
		Logger::info("Setting up openGL callback.");
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
		Logger::warn("glDebugMessageCallback not supported");
#endif
	ctx.backbuffer = std::make_shared<GLBackBuffer>(width, height);
}

void GraphicBackend::destroy()
{
	ctx.backbuffer.reset();
	glFinish();
	glBindVertexArray(0);
}

GraphicApi GraphicBackend::api()
{
	return GraphicApi::OpenGL;
}

void GraphicBackend::resize(uint32_t width, uint32_t height)
{
	ctx.backbuffer->resize(width, height);
}

void GraphicBackend::frame()
{
}

void GraphicBackend::present()
{
	glfwSwapBuffers(PlatformBackend::getGLFW3Handle());
}

void GraphicBackend::viewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	glViewport(x, y, width, height);
}

Framebuffer::Ptr GraphicBackend::backbuffer()
{
	return ctx.backbuffer;
}

void GraphicBackend::render(RenderPass& pass)
{
	{
		// Set framebuffer
		if (pass.framebuffer != ctx.backbuffer)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, ((GLFramebuffer*)pass.framebuffer.get())->getFramebufferID());
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
		if (pass.cull == CullMode::None)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
			switch (pass.cull)
			{
			case CullMode::FrontFace:
				glCullFace(GL_FRONT);
				glFrontFace(GL_CCW);
				break;
			case CullMode::BackFace:
				glCullFace(GL_BACK);
				glFrontFace(GL_CCW);
				break;
			case CullMode::AllFace:
				glCullFace(GL_FRONT_AND_BACK);
				glFrontFace(GL_CCW);
				break;
			}
		}
	}

	{
		// Depth
		if (pass.depth == DepthCompare::None)
		{
			glDisable(GL_DEPTH_TEST);
		}
		else
		{
			glEnable(GL_DEPTH_TEST);
			switch (pass.depth)
			{
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
		// Viewport
		glViewport(
			static_cast<GLint>(pass.viewport.x),
			static_cast<GLint>(pass.viewport.y),
			static_cast<GLint>(pass.viewport.w),
			static_cast<GLint>(pass.viewport.h)
		);
	}

	{
		// TODO Scissor
	}

	{
		// Shader
		pass.shader->use();
		if (pass.texture != nullptr)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, (GLuint)pass.texture->handle().value());
		}
	}
	{
		// Mesh
		pass.mesh->draw(pass.indexCount, pass.indexOffset);
	}
}

void GraphicBackend::screenshot(const Path& path)
{
	glFinish();
	Image image;
	image.width = ctx.backbuffer->width();
	image.height = ctx.backbuffer->height();
	image.bytes.resize(image.width * image.height * 4);
	std::vector<uint8_t> bytes(image.bytes.size());
	glReadPixels(0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, bytes.data());
	uint32_t stride = 4 * image.width;
	for (uint32_t y = 0; y < image.height; y++)
		memcpy(image.bytes.data() + stride * y, bytes.data() + image.bytes.size() - stride - stride * y, stride);
	image.save("./output.jpg");
}

void GraphicBackend::vsync(bool enabled)
{
	if (enabled)
		glfwSwapInterval(1);
	else
		glfwSwapInterval(0);
	ctx.vsync = enabled;
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

Texture::Ptr GraphicBackend::createTexture(uint32_t width, uint32_t height, Texture::Format format, const uint8_t* data, Sampler sampler)
{
	return std::make_shared<GLTexture>(width, height, format, data, sampler, false);
}

Framebuffer::Ptr GraphicBackend::createFramebuffer(uint32_t width, uint32_t height, Framebuffer::AttachmentType* attachment, size_t count, Sampler sampler)
{
	return std::make_shared<GLFramebuffer>(width, height, attachment, count, sampler);
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
		std::cerr << str << std::endl;
		throw std::runtime_error(str);
	}
	return ShaderID(shaderID);
}

Shader::Ptr GraphicBackend::createShader(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Attributes>& attributes)
{
	return std::make_shared<GLShader>(vert, frag, compute, attributes);
}

};
#endif