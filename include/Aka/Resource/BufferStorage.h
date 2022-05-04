#pragma once

#include <Aka/Resource/Storage.h>
#include <Aka/Resource/ResourceAllocator.h>

#include <Aka/Graphic/Buffer.h>

namespace aka {

struct Buffer {
	const gfx::Buffer* buffer;
};

using BufferAllocator = ResourceAllocator<Buffer>;

struct BufferStorage : IStorage<Buffer>
{
	static constexpr uint16_t major = 0;
	static constexpr uint16_t minor = 1;

	gfx::BufferType type;
	gfx::BufferUsage usage;
	gfx::BufferCPUAccess access;
	std::vector<uint8_t> bytes;

	bool load(const Path& path) override;
	bool save(const Path& path) const override;

	Buffer* allocate() const override;
	void deallocate(Buffer* buffer) const override;
	void serialize(const Buffer& mesh) override;

	size_t size(const Buffer& mesh) override;
};

};