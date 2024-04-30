#include <Aka/Memory/Allocator/DefaultAllocator.h>

namespace aka {

DefaultAllocator::DefaultAllocator(const char* name, AllocatorMemoryType memoryType, AllocatorCategory category, Allocator* parent, size_t blockSize) :
	Allocator(name, memoryType, category)
{
}

void* DefaultAllocator::allocate_internal(size_t size, AllocatorFlags flags)
{
	return Memory::alloc(size);
}
void* DefaultAllocator::alignedAllocate_internal(size_t size, size_t alignement, AllocatorFlags flags)
{
	return Memory::allocAlligned(alignement, size);
}

void DefaultAllocator::deallocate_internal(void* elements)
{
	Memory::free(elements);
}

void DefaultAllocator::alignedDeallocate_internal(void* elements)
{
	Memory::freeAligned(elements);
}

void* DefaultAllocator::reallocate_internal(void* elements, size_t size, AllocatorFlags flags)
{
	return Memory::realloc(elements, size);
}

void* DefaultAllocator::alignedReallocate_internal(void* elements, size_t size, size_t alignment, AllocatorFlags flags)
{
	return Memory::reallocAligned(elements, alignment, size);
}

};
