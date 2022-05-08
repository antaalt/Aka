#pragma once

#include <stdint.h>

#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

enum class BufferType : uint8_t
{
	Vertex,
	Index,
	Uniform,
	ShaderStorage,
};

enum class BufferUsage : uint8_t
{
	Default, // Read & write access GPU
	Immutable, // GPU Read only, no CPU access
	Dynamic, // GPU Read only CPU write only
	Staging, // Support data transfer
};

enum class BufferCPUAccess : uint8_t
{
	None = 0,
	Read = (1 << 0),
	Write = (1 << 1),
	ReadWrite = Read | Write
};

enum class BufferMap : uint8_t
{
	Read,
	Write,
	ReadWrite,
	WriteDiscard,
	WriteNoOverwrite
};

struct Buffer;
using BufferHandle = ResourceHandle<Buffer>;

struct Buffer : Resource
{
	uint32_t size;

	BufferType type;
	BufferUsage usage;
	BufferCPUAccess access;

	static BufferHandle createIndexBuffer(uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data = nullptr);
	static BufferHandle createVertexBuffer(uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data = nullptr);
	static BufferHandle createUniformBuffer(uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data = nullptr);
	static BufferHandle createStagingBuffer(uint32_t size, const void* data = nullptr);
	static void destroy(BufferHandle buffer);
};

};
};