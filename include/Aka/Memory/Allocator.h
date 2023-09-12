#pragma once

#include <Aka/Core/Config.h>

namespace aka {

enum class AllocatorFlags {
	None,
};


struct MemoryBlock
{
	MemoryBlock(void* mem, size_t _size);
	~MemoryBlock();

	void* mem;
	size_t size;

	MemoryBlock* prev;
	MemoryBlock* next;
};

class Allocator
{
public:
	Allocator();
	Allocator(Allocator* parent, size_t blockSize);
	virtual ~Allocator();

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
	void request();
	void release();
protected:
	Allocator* m_parent; // parent from which memory was acquired.
	MemoryBlock* m_memory; // allocation for the allocator. if not enough memory, can request more to parent.
};

// Allocator which can create memory
class MemoryAllocator : public Allocator
{
public:
	MemoryAllocator() : Allocator() {}
	virtual ~MemoryAllocator() {}

	void* allocate(size_t size, AllocatorFlags flags = AllocatorFlags::None) override;
	void* alignedAllocate(size_t size, size_t alignement, AllocatorFlags flags = AllocatorFlags::None) override;
	void deallocate(void* elements, size_t size) override;
	void alignedDeallocate(void* elements, size_t size) override;
};

};
