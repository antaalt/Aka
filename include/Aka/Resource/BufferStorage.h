#pragma once

#include <Aka/Resource/Storage.h>
#include <Aka/Resource/ResourceAllocator.h>

#include <Aka/Graphic/Buffer.h>

namespace aka {

using BufferAllocator = ResourceAllocator<gfx::Buffer>;

struct BufferStorage : IStorage<gfx::Buffer>
{
	static constexpr uint16_t major = 0;
	static constexpr uint16_t minor = 1;

	gfx::BufferType type;
	gfx::BufferUsage usage;
	gfx::BufferCPUAccess access;
	std::vector<uint8_t> bytes;

	bool load(const Path& path) override;
	bool save(const Path& path) const override;

	gfx::Buffer* allocate() const override;
	void deallocate(gfx::Buffer* buffer) const override;
	void serialize(const gfx::Buffer* mesh) override;

	size_t size(const gfx::Buffer* mesh) override;
};

};