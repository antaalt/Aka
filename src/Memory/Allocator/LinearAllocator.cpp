#include <Aka/Memory/Allocator/LinearAllocator.h>

namespace aka {

LinearAllocator::LinearAllocator(Allocator* parent, size_t blockSize) :
	Allocator(parent, blockSize),
	m_offset(0)
{

}

void* LinearAllocator::allocate(size_t size, AllocatorFlags flags)
{
	if (size + m_offset > m_memory->size)
		request(); // Out of memory
	size_t offset = m_offset;
	m_offset += size;
	return static_cast<char*>(m_memory->mem) + offset;
}
void* LinearAllocator::alignedAllocate(size_t size, size_t alignement, AllocatorFlags flags)
{
	if (size + m_offset > m_memory->size)
		request(); // Out of memory
	size_t address = alignAdjustment((uintptr_t)m_memory->mem, alignement);

	AKA_NOT_IMPLEMENTED;
	return nullptr;
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
