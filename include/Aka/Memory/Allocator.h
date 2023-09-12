#pragma once

#include <Aka/Core/Config.h>

namespace aka {

enum class AllocatorFlags {
	None,

};


struct MemoryBlock
{
	MemoryBlock(size_t _size);
	~MemoryBlock();
	void* mem;
	size_t size;
};


class Allocator
{
public:
	Allocator(const MemoryBlock& memory) : m_memory(memory) {}
	virtual ~Allocator() {}

	// Allocate memory from the allocator
	virtual void* allocate(size_t size, AllocatorFlags flags = AllocatorFlags::None) = 0;
	// Allocate aligned memory from the allocator
	virtual void* alignedAllocate(size_t size, size_t alignement, AllocatorFlags flags = AllocatorFlags::None) = 0;
	// Deallocate memory from the allocator
	virtual void deallocate(void* elements, size_t size) = 0;
	// Deallocate aligned memory from the allocator
	virtual void alignedDeallocate(void* elements, size_t size) = 0;

	// Find next aligned address for given one.
	static uintptr_t align(uintptr_t address, size_t alignment);
	// Find next aligned address adjustment for given one.
	static size_t alignAdjustment(uintptr_t address, size_t alignment);
protected:
	const MemoryBlock& m_memory;
};

};
