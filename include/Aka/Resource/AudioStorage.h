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

	AudioStream* allocate() const override;
	void deallocate(AudioStream* buffer) const override;
	void serialize(const AudioStream* mesh) override;

	size_t size(const AudioStream* mesh) override;
};

};