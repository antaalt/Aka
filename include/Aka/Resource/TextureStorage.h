#pragma once

#include <Aka/Resource/Storage.h>
#include <Aka/Resource/ResourceAllocator.h>
#include <Aka/OS/Image.h>

#include <Aka/Graphic/Texture.h>

namespace aka {

struct Texture
{
	gfx::TextureHandle texture;
};

using TextureAllocator = ResourceAllocator<Texture>;

struct TextureStorage : IStorage<Texture>
{
	static constexpr uint16_t major = 0;
	static constexpr uint16_t minor = 2;

	gfx::TextureType type;
	gfx::TextureFormat format;
	gfx::TextureFlag flags;
	uint32_t levels;
	
	std::vector<Image> images;

	bool load(const Path& path) override;
	bool save(const Path& path) const override;

	Texture* allocate() const override;
	void deallocate(Texture* texture) const override;
	void serialize(const Texture& texture) override;

	size_t size(const Texture& texture) override;
};

};