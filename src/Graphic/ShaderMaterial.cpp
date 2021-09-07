#include <Aka/Graphic/ShaderMaterial.h>

#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/OS/Logger.h>

#include <cstring>

namespace aka {

ShaderMaterial::ShaderMaterial(Shader::Ptr shader) :
	m_shader(shader)
{
}

ShaderMaterial::~ShaderMaterial()
{
}

ShaderMaterial::Ptr ShaderMaterial::create(Shader::Ptr shader)
{
	return GraphicBackend::createShaderMaterial(shader);
}

Shader::Ptr ShaderMaterial::shader()
{
	return m_shader;
}

void ShaderMaterial::set(const char* name, const Buffer::Ptr& buffer)
{
	set(name, &buffer, 1);
}

void ShaderMaterial::set(const char* name, const Buffer::Ptr* buffers, size_t count)
{
	uint32_t bindPoint = 0;
	for (const Uniform& uniform : *m_shader)
	{
		if (uniform.type != UniformType::Buffer)
			continue;
		if (uniform.name == name)
		{
			for (size_t iBuf = 0; iBuf < count; iBuf++)
				m_buffers[bindPoint + iBuf] = buffers[iBuf];
			return;
		}
		bindPoint += uniform.count;
	}
	Logger::error("Buffer not found : ", name);
}

void ShaderMaterial::set(const char* name, const Texture::Ptr& texture)
{
	set(name, &texture, 1);
}

void ShaderMaterial::set(const char* name, const Texture::Ptr* textures, size_t count)
{
	// Texture slot is attributed "automatically" (first called, first served). fix this.
	uint32_t slot = 0;
	for (const Uniform& uniform : *m_shader)
	{
		if (uniform.type != UniformType::Texture2D && uniform.type != UniformType::TextureCubemap)
			continue;
		if (uniform.name == name)
		{
			for (size_t iTex = 0; iTex < count; iTex++)
				m_textures[slot + iTex] = textures[iTex];
			return;
		}
		slot += uniform.count;
	}
	Logger::error("Texture not found : ", name);
}

void ShaderMaterial::set(const char* name, TextureSampler sampler)
{
	set(name, &sampler, 1);
}

void ShaderMaterial::set(const char* name, const TextureSampler* samplers, size_t count)
{
	uint32_t slot = 0;
	for (const Uniform& uniform : *m_shader)
	{
		if (uniform.type != UniformType::Texture2D && uniform.type != UniformType::TextureCubemap)
			continue;
		if (uniform.name == name)
		{
			for (size_t iTex = 0; iTex < count; iTex++)
				m_samplers[slot + iTex] = samplers[iTex];
			return;
		}
		slot += 1 * uniform.count;
	}
	Logger::error("Texture not found : ", name);
}

};