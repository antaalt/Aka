#pragma once

#include <stdint.h>
#include <memory>

#include <Aka/Core/StrictType.h>
#include <Aka/Core/Geometry.h>

namespace aka {

enum class BufferType {
	Vertex,
	Index,
	Uniform,
	ShaderStorage,
	/*AtomicCounter,
	CopyRead,
	CopyWrite,
	DispatchIndirect,
	DrawIndirect,
	PixelPack,
	PixelUnpack,
	Query,
	Texture,
	TransformFeedback,*/
};

enum class BufferUsage {
	Default, // Read & write access GPU
	Immutable, // GPU Read only, no CPU access
	Dynamic, // GPU Read only CPU write only
	Staging, // Support data transfer
};

enum class BufferCPUAccess {
	None = 0,
	Read = (1 << 0),
	Write = (1 << 1),
	ReadWrite = Read | Write
};

enum class BufferMap {
	Read,
	Write,
	ReadWrite,
	WriteDiscard,
	WriteNoOverwrite
};

class Buffer
{
public:
	using Ptr = std::shared_ptr<Buffer>;
	using Handle = StrictType<uintptr_t, struct BufferHandleTag>;
protected:
	Buffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access);
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	virtual ~Buffer();
public:
	static Buffer::Ptr create(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data = nullptr);

	BufferType type() const;

	size_t size() const;

	BufferUsage usage() const;

	BufferCPUAccess access() const;

	virtual void reallocate(size_t size, const void* data = nullptr) = 0;

	virtual void upload(const void* data, size_t size, size_t offset = 0) = 0;

	virtual void upload(const void* data) = 0;

	virtual void download(void* data, size_t size, size_t offset = 0) = 0;

	virtual void download(void* data) = 0;

	virtual void* map(BufferMap map) = 0;

	virtual void unmap() = 0;

	virtual Handle handle() const = 0;

protected:
	size_t m_size;
	BufferType m_type;
	BufferUsage m_usage;
	BufferCPUAccess m_access;
};

struct SubBuffer {
	Buffer::Ptr buffer;
	uint32_t offset;
	uint32_t size;
};

}
