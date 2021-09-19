#pragma once

#include <Aka/Graphic/Program.h>

namespace aka {

class Material
{
public:
	using Ptr = std::shared_ptr<Material>;
protected:
	Material(Program::Ptr program);
	Material(const Material&) = delete;
	const Material& operator=(const Material&) = delete;
	virtual ~Material();
public:
	static Material::Ptr create(Program::Ptr program);

	Program::Ptr program();

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
	Program::Ptr m_program;
	std::vector<Buffer::Ptr> m_buffers;
	std::vector<Texture::Ptr> m_textures;
	std::vector<TextureSampler> m_samplers;
};

}