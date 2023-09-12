#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Memory/Allocator.h>
#include <Aka/OS/Logger.h>

namespace aka {

// Allocator should not be malloc dependent, that is another issue.
class DefaultAllocator final : public Allocator
{
public:
	void* allocate(size_t size, AllocatorFlags flags = AllocatorFlags::None) override;
	void* alignedAllocate(size_t size, size_t alignement, AllocatorFlags flags = AllocatorFlags::None) override;
	void deallocate(void* elements, size_t size) override;
	void alignedDeallocate(void* elements, size_t size) override;
};


inline void* DefaultAllocator::allocate(size_t size, AllocatorFlags flags)
{
	return Memory::alloc(size);
}
inline void* DefaultAllocator::alignedAllocate(size_t size, size_t alignement, AllocatorFlags flags)
{
	return Memory::alignedAlloc(alignement, size);
}

inline void DefaultAllocator::deallocate(void* elements, size_t size)
{
	// size should be the same than call for allocate.
	Memory::free(elements);
}

inline void DefaultAllocator::alignedDeallocate(void* elements, size_t size)
{
	Memory::alignedFree(elements);
}

};
