#pragma once

#include <Aka/Core/Config.h>

#include <Aka/Memory/Allocator/LinearAllocator.h>
#include <Aka/Memory/Allocator/DebugAllocator.h>

namespace aka {

using byte_t = uint8_t;

class Memory
{
public:
	Memory(size_t size);
	Memory(const Memory& memory);
	Memory& operator=(const Memory& memory);
	Memory(Memory&& memory);
	Memory& operator=(Memory&& memory);
	~Memory();

public:
	// Get the size of the memory block
	size_t size() const;
	// Get the raw pointer to it
	void* raw() const;

public:
	// Allocate memory for given size
	static void* alloc(size_t size);
	// Allocate aligned memory for given size
	static void* alignedAlloc(size_t alignment, size_t size);
	// Deallocate memory
	static void (free)(void* data);
	// Deallocate aligned memory
	static void (alignedFree)(void* data);
	// Reallocate memory
	static void* (realloc)(void* data, size_t size);
	// Reallocate aligned memory
	static void* (alignedRealloc)(void* data, size_t alignment, size_t size);
	// Copy memory 
	static void* copy(void* dst, const void* src, size_t count);
	// Set memory
	static void set(void* memory, uint8_t data, size_t size);
	// Compare memory
	static int32_t compare(const void* lhs, const void* rhs, size_t size);
	// Set memory to zero
	static void zero(void* memory, size_t size);
	// Compare memories
	bool operator==(const Memory& rhs);
	// Compare memories
	bool operator!=(const Memory& rhs);
private:
	size_t m_size;
	void* m_data;
};

// Default allocator for whole app.
namespace mem {

// Until we create a better allocator, use linear
using DefaultAllocatorType = DebugAllocator<LinearAllocator>;

static MemoryBlock GfxMemoryBlock(1 << 16);
static MemoryBlock AudioMemoryBlock(1 << 16);
static MemoryBlock DefaultMemoryBlock(1 << 26);

static DefaultAllocatorType GfxAllocator(GfxMemoryBlock);
static DefaultAllocatorType AudioAllocator(AudioMemoryBlock);
static DefaultAllocatorType DefaultAllocator(DefaultMemoryBlock);

}

};
