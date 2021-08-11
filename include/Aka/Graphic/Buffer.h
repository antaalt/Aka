#pragma once

#include <stdint.h>
#include <memory>

#include <Aka/Core/StrictType.h>
#include <Aka/Core/Geometry.h>

namespace aka {

enum class BufferType {
	Array, // vertices
	AtomicCounter,
	CopyRead,
	CopyWrite,
	DispatchIndirect,
	DrawIndirect,
	ElementArray,
	PixelPack,
	PixelUnpack,
	Query,
	ShaderStorage,
	Texture,
	TransformFeedback,
	Uniform
};

enum class BufferUsage {
	Stream,
	Static,
	Dynamic,
};

enum class BufferAccess {
	ReadOnly,
	WriteOnly,
	ReadAndWrite,
};

class Buffer
{
public:
	using Ptr = std::shared_ptr<Buffer>;
	using Handle = StrictType<uintptr_t, struct BufferHandleTag>;
protected:
	Buffer(BufferType type, size_t size, BufferUsage usage, BufferAccess access);
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	virtual ~Buffer();
public:
	static Buffer::Ptr create(BufferType type, size_t size, BufferUsage usage, BufferAccess access, const void* data = nullptr);

	BufferType type() const;

	size_t size() const;

	BufferUsage usage() const;

	BufferAccess access() const;

	virtual void reallocate(size_t size, const void* data = nullptr) = 0;

	virtual void upload(const void* data, size_t size, size_t offset = 0) = 0;

	virtual void upload(const void* data) = 0;

	virtual void download(void* data, size_t size, size_t offset = 0) = 0;

	virtual void download(void* data) = 0;

	virtual void* map(BufferAccess access) = 0;

	virtual void unmap() = 0;

	virtual Handle handle() const = 0;

protected:
	size_t m_size;
	BufferType m_type;
	BufferUsage m_usage;
	BufferAccess m_access;
};

struct SubBuffer {
	Buffer::Ptr buffer;
	uint32_t offset;
	uint32_t size;
};

}
