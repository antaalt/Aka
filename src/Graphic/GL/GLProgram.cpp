#if defined(AKA_USE_OPENGL)
#include "GLProgram.h"

#include "GLContext.h"

#include <Aka/OS/Logger.h>

namespace aka {

GLShader::GLShader(GLuint shaderID) :
	Shader(),
	m_shaderID(shaderID)
{
}
GLShader::~GLShader()
{
	if (m_shaderID != 0)
		glDeleteShader(m_shaderID);
}

Shader::Ptr GLShader::compileGL(const char* content, ShaderType type)
{
	GLuint shaderID = glCreateShader(glType(type));
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
		return nullptr;
	}
	return std::make_shared<GLShader>(shaderID);
}

GLProgram::GLProgram(Shader::Ptr vertex, Shader::Ptr fragment, Shader::Ptr geometry, Shader::Ptr compute, const VertexAttribute* attributes, size_t count) :
	Program(attributes, count)
{
	auto getShaderID = [](Shader* shader) -> GLuint {
		if (shader == nullptr)
			return 0;
		return reinterpret_cast<GLShader*>(shader)->m_shaderID;
	};
	GLuint vert = getShaderID(vertex.get());
	GLuint frag = getShaderID(fragment.get());
	GLuint comp = getShaderID(compute.get());
	GLuint geo = getShaderID(geometry.get());
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
GLProgram::~GLProgram()
{
	if (m_programID != 0)
		glDeleteProgram(m_programID);
}

};

#endif