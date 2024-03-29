#include <Aka/Memory/Allocator/LinearAllocator.h>

namespace aka {

LinearAllocator::LinearAllocator(const char* name, Allocator* parent, size_t blockSize) :
	Allocator(name, parent, blockSize),
	m_offset(0)
{

}

LinearAllocator::~LinearAllocator()
{

}

void* LinearAllocator::allocate(size_t size, AllocatorFlags flags)
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
void* LinearAllocator::alignedAllocate(size_t size, size_t alignement, AllocatorFlags flags)
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

void LinearAllocator::deallocate(void* address, size_t size)
{
	// Linear allocator does not need to deallocate.
}

void LinearAllocator::alignedDeallocate(void* address, size_t size)
{
	// Linear allocator does not need to deallocate.
}

};
