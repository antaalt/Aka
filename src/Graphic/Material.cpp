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
	return GraphicBackend::device()->createMaterial(program);
}

Program::Ptr Material::program()
{
	return m_program;
}

void Material::set(const char* name, const Buffer::Ptr& buffer, uint32_t binding)
{
	set(name, &buffer, 1, binding);
}

void Material::set(const char* name, const Buffer::Ptr* buffers, size_t count, size_t offset)
{
	for (const Uniform& uniform : *m_program)
	{
		if (uniform.type != UniformType::Buffer)
			continue;
		if (uniform.name == name)
		{
			AKA_ASSERT(uniform.count >= offset + count, "Invalid range");
			for (size_t i = 0; i < count; i++)
				m_buffers[uniform.binding + offset + i] = buffers[i];
			return;
		}
	}
	Logger::error("Buffer not found : ", name);
}

void Material::set(const char* name, const Texture::Ptr& texture, uint32_t binding)
{
	set(name, &texture, 1, binding);
}

void Material::set(const char* name, const Texture::Ptr* textures, size_t count, size_t offset)
{
	for (const Uniform& uniform : *m_program)
	{
		if (uniform.type != UniformType::Texture2D && uniform.type != UniformType::Texture2DMultisample && uniform.type != UniformType::TextureCubemap)
			continue;
		if (uniform.name == name)
		{
			AKA_ASSERT(uniform.count >= offset + count, "Invalid range");
			for (size_t i = 0; i < count; i++)
			{
				AKA_ASSERT((textures[i]->flags() & TextureFlag::ShaderResource) == TextureFlag::ShaderResource, "Input must be a shader resource");
				m_textures[uniform.binding + offset + i] = textures[i];
			}
			return;
		}
	}
	Logger::error("Texture not found : ", name);
}

void Material::set(const char* name, const TextureSampler& sampler, uint32_t binding)
{
	set(name, &sampler, 1, binding);
}

void Material::set(const char* name, const TextureSampler* samplers, size_t count, size_t offset)
{
	for (const Uniform& uniform : *m_program)
	{
		if (uniform.type != UniformType::Texture2D && uniform.type != UniformType::Texture2DMultisample && uniform.type != UniformType::TextureCubemap)
			continue;
		if (uniform.name == name)
		{
			AKA_ASSERT(uniform.count >= offset + count, "Invalid range");
			for (size_t i = 0; i < count; i++)
				m_samplers[uniform.binding + offset + i] = samplers[i];
			return;
		}
	}
	Logger::error("Sampler not found : ", name);
}

};