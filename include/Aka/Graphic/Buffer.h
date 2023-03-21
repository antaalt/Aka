#pragma once

#include <stdint.h>

#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

enum class BufferType : uint8_t
{
	Unknown,

	Vertex,
	Index,
	Uniform,
	Storage,
	Indirect,

	First = Vertex,
	Last = Indirect,
};

enum class BufferUsage : uint8_t
{
	Unknown,

	Default, // Read & write access GPU
	Immutable, // GPU Read only, no CPU access
	Dynamic, // GPU Read only CPU write only
	Staging, // Support data transfer

	First = Default,
	Last = Staging,
};

enum class BufferCPUAccess : uint8_t
{
	None		= 0,
	Read		= 1 << 0,
	Write		= 1 << 1,
	ReadWrite	= Read | Write
};

enum class BufferMap : uint8_t
{
	Unknown,

	Read,
	Write,
	ReadWrite,
	WriteDiscard,
	WriteNoOverwrite,

	First = Read,
	Last = WriteNoOverwrite,
};

struct Buffer;
using BufferHandle = ResourceHandle<Buffer>;

struct Buffer : Resource
{
	Buffer(const char* name, BufferType type, uint32_t size, BufferUsage usage, BufferCPUAccess access);

	uint32_t size;

	BufferType type;
	BufferUsage usage;
	BufferCPUAccess access;

	static BufferHandle createIndexBuffer(const char* name, uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data = nullptr);
	static BufferHandle createVertexBuffer(const char* name, uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data = nullptr);
	static BufferHandle createUniformBuffer(const char* name, uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data = nullptr);
	static BufferHandle createStagingBuffer(const char* name, uint32_t size, const void* data = nullptr);
	static void destroy(BufferHandle buffer);
};

};
};