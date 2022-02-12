#pragma once

#include <Aka/Resource/Storage.h>
#include <Aka/Resource/ResourceAllocator.h>
#include <Aka/OS/Image.h>

#include <Aka/Graphic/Texture.h>

namespace aka {

using TextureAllocator = ResourceAllocator<Texture>;

struct TextureStorage : IStorage<Texture>
{
	static constexpr uint16_t major = 0;
	static constexpr uint16_t minor = 2;

	TextureType type;
	TextureFormat format;
	TextureFlag flags;
	
	std::vector<Image> images;

	bool load(const Path& path) override;
	bool save(const Path& path) const override;

	Texture* allocate() const override;
	void deallocate(Texture* texture) const override;
	void serialize(const Texture* texture) override;

	size_t size(const Texture* texture) override;
};

};