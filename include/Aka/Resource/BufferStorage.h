#pragma once

#include <Aka/Resource/Storage.h>
#include <Aka/Resource/ResourceAllocator.h>

#include <Aka/Graphic/Buffer.h>

namespace aka {

using BufferAllocator = ResourceAllocator<Buffer>;

struct BufferStorage : IStorage<Buffer>
{
	static constexpr uint16_t major = 0;
	static constexpr uint16_t minor = 1;

	BufferType type;
	BufferUsage usage;
	BufferCPUAccess access;
	std::vector<uint8_t> bytes;

	bool load(const Path& path) override;
	bool save(const Path& path) const override;

	std::shared_ptr<Buffer> to() const override;
	void from(const std::shared_ptr<Buffer>& mesh) override;

	size_t size(const std::shared_ptr<Buffer>& mesh) override;
};

};