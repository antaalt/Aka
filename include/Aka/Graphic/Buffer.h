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

using BufferHandle = StrictType<uintptr_t, struct BufferHandleTag>;

class Buffer
{
public:
	using Ptr = std::shared_ptr<Buffer>;
protected:
	Buffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access);
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	virtual ~Buffer();
public:
	// Create a buffer
	static Buffer::Ptr create(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data = nullptr);

	// Get the buffer type
	BufferType type() const;
	// Get the buffer size
	size_t size() const;
	// Get the buffer usage
	BufferUsage usage() const;
	// Get the buffer access for CPU
	BufferCPUAccess access() const;

	// Reallocate the buffer memory
	virtual void reallocate(size_t size, const void* data = nullptr) = 0;
	// Upload partial data to the buffer
	virtual void upload(const void* data, size_t size, size_t offset = 0) = 0;
	// Upload data to the whole buffer
	virtual void upload(const void* data) = 0;
	// Download partial data from buffer
	virtual void download(void* data, size_t size, size_t offset = 0) = 0;
	// Download data from whole buffer
	virtual void download(void* data) = 0;

	// Map the buffer on CPU
	virtual void* map(BufferMap map) = 0;
	// Unmap the buffer
	virtual void unmap() = 0;

	// Get a native handle to the buffer
	virtual BufferHandle handle() const = 0;

	// Copy the buffer to another buffer
	void copy(const Buffer::Ptr& dst);
	// Copy a partial buffer to another buffer
	virtual void copy(const Buffer::Ptr& dst, size_t offsetSRC, size_t offsetDST, size_t size) = 0;

protected:
	size_t m_size; // Size of the buffer
	BufferType m_type; // Type of the buffer
	BufferUsage m_usage; // Usage of the buffer for memory
	BufferCPUAccess m_access; // Access of the buffer on the CPU
};

struct SubBuffer {
	Buffer::Ptr buffer;
	uint32_t offset;
	uint32_t size;
};

}
