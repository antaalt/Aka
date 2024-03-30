#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Memory/AllocatorTracker.hpp>
#include <Aka/Memory/AllocatorTypes.hpp>

namespace aka {



struct MemoryBlock
{
	MemoryBlock(void* mem, size_t _size);
	~MemoryBlock();

	void* mem;
	size_t size;

	MemoryBlock* next;
};

class Allocator
{
public:
	Allocator(const char* name, AllocatorMemoryType memoryType, AllocatorCategory category);
	Allocator(const char* name, AllocatorMemoryType memoryType, AllocatorCategory category, Allocator* parent, size_t blockSize);
	virtual ~Allocator();

	// Allocate memory from the allocator & track allocations
	template <typename T>
	T* allocate(size_t count, AllocatorFlags flags = AllocatorFlags::None);
	// Deallocate memory from the allocator & track allocations
	template <typename T>
	void deallocate(T* elements, size_t count);
protected:
	// Allocate memory from the allocator
	virtual void* allocate_internal(size_t size, AllocatorFlags flags = AllocatorFlags::None) = 0;
	// Allocate aligned memory from the allocator
	virtual void* alignedAllocate_internal(size_t size, size_t alignement, AllocatorFlags flags = AllocatorFlags::None) = 0;
	// Deallocate memory from the allocator
	virtual void deallocate_internal(void* elements, size_t size) = 0;
	// Deallocate aligned memory from the allocator
	virtual void alignedDeallocate_internal(void* elements, size_t size) = 0;

	// Find next aligned address for given one.
	static uintptr_t align(uintptr_t address, size_t alignment);
	// Find next aligned address adjustment for given one.
	static size_t alignAdjustment(uintptr_t address, size_t alignment);

protected:

	const char* getName() const;
	// Get last memory block allocated
	MemoryBlock* getMemoryBlock();
	// Get parent allocator
	Allocator* getParentAllocator();
	// Request a new memory block allocation by parent allocator
	MemoryBlock* requestNewMemoryBlock();
	// Release all memory block & call parent allocator to free memory.
	void releaseAllMemoryBlocks();
private:
	char m_name[32];
	AllocatorMemoryType m_type;
	AllocatorCategory m_category;
	Allocator* m_parent; // parent from which memory was acquired.
	MemoryBlock* m_memory; // allocation for the allocator. if not enough memory, can request more to parent.
};


template <typename T>
T* Allocator::allocate(size_t count, AllocatorFlags flags)
{
#if defined(AKA_TRACK_MEMORY_ALLOCATIONS)
	AllocatorTracker& tracker = getAllocatorTracker();
#endif
	void* data = allocate_internal(count * sizeof(T), flags);
#if defined(AKA_TRACK_MEMORY_ALLOCATIONS)
	tracker.allocate<T>(data, count, m_type, m_category);
#endif
	return static_cast<T*>(data);
}

template <typename T>
void Allocator::deallocate(T* elements, size_t count)
{
#if defined(AKA_TRACK_MEMORY_ALLOCATIONS)
	AllocatorTracker& tracker = getAllocatorTracker();
	tracker.deallocate(elements, m_type, m_category);
#endif
	deallocate_internal(static_cast<void*>(elements), count * sizeof(T));
}

};
