#include <Aka/Graphic/Material.h>

#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/OS/Logger.h>

#include <cstring>

namespace aka {

Material::Material(Program::Ptr program) :
	m_program(program)
{
}

Material::~Material()
{
}

Material::Ptr Material::create(Program::Ptr program)
{
	return GraphicBackend::createMaterial(program);
}

Program::Ptr Material::program()
{
	return m_program;
}

void Material::set(const char* name, const Buffer::Ptr& buffer)
{
	set(name, &buffer, 1);
}

void Material::set(const char* name, const Buffer::Ptr* buffers, size_t count)
{
	for (const Uniform& uniform : *m_program)
	{
		if (uniform.type != UniformType::Buffer)
			continue;
		if (uniform.name == name)
		{
			for (size_t i = 0; i < count; i++)
				m_buffers[uniform.binding + i] = buffers[i];
			return;
		}
	}
	Logger::error("Buffer not found : ", name);
}

void Material::set(const char* name, const Texture::Ptr& texture)
{
	set(name, &texture, 1);
}

void Material::set(const char* name, const Texture::Ptr* textures, size_t count)
{
	for (const Uniform& uniform : *m_program)
	{
		if (uniform.type != UniformType::Texture2D && uniform.type != UniformType::Texture2DMultisample && uniform.type != UniformType::TextureCubemap)
			continue;
		if (uniform.name == name)
		{
			for (size_t i = 0; i < count; i++)
				m_textures[uniform.binding + i] = textures[i];
			return;
		}
	}
	Logger::error("Texture not found : ", name);
}

void Material::set(const char* name, TextureSampler sampler)
{
	set(name, &sampler, 1);
}

void Material::set(const char* name, const TextureSampler* samplers, size_t count)
{
	for (const Uniform& uniform : *m_program)
	{
		if (uniform.type != UniformType::Texture2D && uniform.type != UniformType::Texture2DMultisample && uniform.type != UniformType::TextureCubemap)
			continue;
		if (uniform.name == name)
		{
			for (size_t i = 0; i < count; i++)
				m_samplers[uniform.binding + i] = samplers[i];
			return;
		}
	}
	Logger::error("Sampler not found : ", name);
}

};