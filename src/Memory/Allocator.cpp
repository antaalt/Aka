#include <Aka/Memory/Allocator.h>

#include <Aka/Memory/Memory.h>

namespace aka {

MemoryBlock::MemoryBlock(size_t _size) :
	mem(Memory::alloc(_size)),
	size(_size)
{
}

MemoryBlock::~MemoryBlock()
{
	Memory::free(mem);
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