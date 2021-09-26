#pragma once

#include <Aka/Resource/Storage.h>
#include <Aka/Resource/ResourceAllocator.h>

#include <Aka/Core/Font.h>

namespace aka {

using FontAllocator = ResourceAllocator<Font>;

struct FontStorage : IStorage<Font>
{
	static constexpr uint16_t major = 0;
	static constexpr uint16_t minor = 1;

	std::vector<byte_t> ttf;
	
	bool load(const Path& path) override;
	bool save(const Path& path) const override;

	std::shared_ptr<Font> to() const override;
	void from(const std::shared_ptr<Font>& mesh) override;

	size_t size(const std::shared_ptr<Font>& mesh) override;
};

};