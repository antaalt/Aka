#if defined(AKA_USE_OPENGL)
#include "GLMaterial.h"

#include "GLContext.h"
#include "GLProgram.h"

#include <Aka/OS/Logger.h>

namespace aka {

GLMaterial::GLMaterial(Program::Ptr shader) :
	Material(shader)
{
	// TODO This is API agnostic, move in Material.cpp
	GLint activeUniforms = 0;
	uint32_t textureCount = 0;
	uint32_t imageCount = 0;
	uint32_t bufferCount = 0;
	for (const Uniform& uniform : *m_program)
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

GLMaterial::~GLMaterial()
{
}

void GLMaterial::use() const
{
	GLProgram* glProgram = reinterpret_cast<GLProgram*>(m_program.get());
	glUseProgram(glProgram->m_programID);
	for (const Uniform& uniform : *m_program)
	{
		// TODO map this for performance
		GLuint programID = glProgram->m_programID;
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
					GLenum type = glType(texture->type());
					glBindTexture(type, (GLuint)texture->handle().value());
					glTexParameteri(type, GL_TEXTURE_MAG_FILTER, glFilter(sampler.filterMag, TextureMipMapMode::None));
					glTexParameteri(type, GL_TEXTURE_MIN_FILTER, glFilter(sampler.filterMin, sampler.mipmapMode));
					glTexParameteri(type, GL_TEXTURE_WRAP_S, glWrap(sampler.wrapU));
					glTexParameteri(type, GL_TEXTURE_WRAP_T, glWrap(sampler.wrapV));
					glTexParameteri(type, GL_TEXTURE_WRAP_R, glWrap(sampler.wrapW));
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
}

};

#endif