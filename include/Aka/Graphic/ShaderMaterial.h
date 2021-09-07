#pragma once

#include "Shader.h"

namespace aka {

class ShaderMaterial
{
public:
	using Ptr = std::shared_ptr<ShaderMaterial>;
protected:
	ShaderMaterial(Shader::Ptr shader);
	ShaderMaterial(const ShaderMaterial&) = delete;
	const ShaderMaterial& operator=(const ShaderMaterial&) = delete;
	virtual ~ShaderMaterial();
public:
	static ShaderMaterial::Ptr create(Shader::Ptr shader);

	Shader::Ptr shader();

	// TODO use buffer handle & texture handle
	void set(const char* name, const Buffer::Ptr& buffer);
	void set(const char* name, const Buffer::Ptr* buffer, size_t count);

	void set(const char* name, const Texture::Ptr& texture);
	void set(const char* name, const Texture::Ptr* texture, size_t count);

	void set(const char* name, TextureSampler sampler);
	void set(const char* name, const TextureSampler* sampler, size_t count);

	// Force a specific slot for a texture
	//void setSlot(const char* name, uint32_t slot);

protected:
	Shader::Ptr m_shader;
	std::vector<Buffer::Ptr> m_buffers;
	std::vector<Texture::Ptr> m_textures;
	std::vector<TextureSampler> m_samplers;
};

}