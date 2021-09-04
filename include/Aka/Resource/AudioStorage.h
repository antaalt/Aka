#pragma once

#include <Aka/Resource/Storage.h>
#include <Aka/Resource/ResourceAllocator.h>

#include <Aka/Audio/AudioStream.h>

namespace aka {

using AudioAllocator = ResourceAllocator<AudioStream>;

struct AudioStorage : IStorage<AudioStream>
{
	static constexpr uint16_t major = 0;
	static constexpr uint16_t minor = 1;


	bool load(const Path& path) override;
	bool save(const Path& path) const override;

	std::shared_ptr<AudioStream> to() const override;
	void from(const std::shared_ptr<AudioStream>& mesh) override;

	size_t size(const std::shared_ptr<AudioStream>& mesh) override;
};

};