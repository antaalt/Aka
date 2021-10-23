#include <Aka/Memory/Allocator.h>

namespace aka {

Allocator::Allocator(void* mem, size_t size) :
	m_mem(mem),
	m_size(size),
	m_used(0)
{
}

Allocator::~Allocator()
{
}

const void* Allocator::mem() const
{
	return m_mem;
}

size_t Allocator::size() const
{
	return m_size;
}

size_t Allocator::used() const
{
	return m_used;
}

uintptr_t Allocator::align(uintptr_t address, size_t alignment)
{
	const size_t mask = alignment - 1;
	AKA_ASSERT((alignment & mask) == 0, "Alignement is not a power of 2.");
	return (address + mask) & ~mask;
}
// Find next aligned address adjustment for given one.
size_t Allocator::alignAdjustment(uintptr_t address, size_t alignment)
{
	const size_t mask = alignment - 1;
	AKA_ASSERT((alignment & mask) == 0, "Alignement is not a power of 2.");
	size_t adjustment = alignment - (address & mask);
	if (adjustment == alignment) return 0; // Already aligned 
	return adjustment;
}

};