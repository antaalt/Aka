#include <Aka/Memory/Allocator/LinearAllocator.h>

namespace aka {

LinearAllocator::LinearAllocator(void* chunk, size_t size) :
	Allocator(chunk, size)
{
}

LinearAllocator::~LinearAllocator()
{
}

void* LinearAllocator::allocate(size_t size, size_t alignement)
{
	uintptr_t current = (uintptr_t)m_mem + m_used;
	size_t adjustment = Allocator::alignAdjustment(current, alignement);
	if (m_used + adjustment + size > m_size)
		return nullptr; // Not enough space in allocator
	uintptr_t aligned = current + adjustment;
	m_used += size + adjustment;
	return (void*)aligned;
}

void LinearAllocator::deallocate(void* address, size_t size)
{
	// Linear allocator does not need to deallocate.
}

void LinearAllocator::reset()
{
	m_used = 0;
}

bool LinearAllocator::contiguous() const
{
	return true;
}

};
