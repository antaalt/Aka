#pragma once

#include <Aka/Resource/Storage.h>
#include <Aka/Resource/ResourceAllocator.h>

#include <Aka/Graphic/Texture.h>

namespace aka {

using TextureAllocator = ResourceAllocator<Texture>;

struct TextureStorage : IStorage<Texture>
{
	static constexpr uint16_t major = 0;
	static constexpr uint16_t minor = 1;

	TextureType type;
	TextureFormat format;
	TextureFlag flags;
	std::vector<Image> images;

	bool load(const Path& path) override;
	bool save(const Path& path) const override;

	std::shared_ptr<Texture> to() const override;
	void from(const std::shared_ptr<Texture>& mesh) override;

	size_t size(const std::shared_ptr<Texture>& mesh) override;
};

};