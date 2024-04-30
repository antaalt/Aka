#include <Aka/Memory/Allocator/LinearAllocator.h>

namespace aka {

LinearAllocator::LinearAllocator(const char* name, AllocatorMemoryType memoryType, AllocatorCategory category, Allocator* parent = nullptr, size_t blockSize = 0) :
	Allocator(name, memoryType, category),
	m_offset(0)
{

}

LinearAllocator::~LinearAllocator()
{

}

void* LinearAllocator::allocate_internal(size_t size, AllocatorFlags flags)
{
	MemoryBlock* block = getMemoryBlock();
	if (size + m_offset > block->size)
	{
		block = requestNewMemoryBlock(); // Out of memory
		m_offset = 0; // Reset offset
	}
	size_t offset = m_offset;
	m_offset += size;
	return static_cast<uint8_t*>(block->mem) + offset;
}
void* LinearAllocator::alignedAllocate_internal(size_t size, size_t alignement, AllocatorFlags flags)
{
	MemoryBlock* block = getMemoryBlock();
	if (size + m_offset > block->size)
	{
		block = requestNewMemoryBlock(); // Out of memory
		m_offset = 0; // Reset offset
	}
	size_t offset = m_offset + alignAdjustment((uintptr_t)block->mem + m_offset, alignement);
	m_offset = offset + size;
	return static_cast<uint8_t*>(block->mem) + offset;
}

void LinearAllocator::deallocate_internal(void* address)
{
	AKA_UNUSED(address);
	// Linear allocator does not need to deallocate.
}

void LinearAllocator::alignedDeallocate_internal(void* address)
{
	AKA_UNUSED(address);
	// Linear allocator does not need to deallocate.
}

void* LinearAllocator::reallocate_internal(void* elements, size_t size, AllocatorFlags flags)
{
	AKA_UNUSED(elements);
	// Linear allocator does not need to deallocate.
	return allocate_internal(size);
}

void* LinearAllocator::alignedReallocate_internal(void* elements, size_t size, size_t alignment, AllocatorFlags flags)
{
	AKA_UNUSED(elements);
	// Linear allocator does not need to deallocate.
	return alignedAllocate_internal(alignment, size);
}


};
