#pragma once

#include <stdint.h>

#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

enum class BufferType : uint8_t
{
	Unknown,

	Vertex = 1 << 0,
	Index = 1 << 1,
	Uniform = 1 << 2,
	Storage = 1 << 3,
	Indirect = 1 << 4,
};
AKA_IMPLEMENT_BITMASK_OPERATOR(BufferType);

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
	virtual ~Buffer() {}

	uint32_t size;

	BufferType type;
	BufferUsage usage;
	BufferCPUAccess access;
};

};
};