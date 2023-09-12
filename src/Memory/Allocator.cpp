#include <Aka/Memory/Allocator.h>

#include <Aka/Memory/Memory.h>

namespace aka {

MemoryBlock::MemoryBlock(void* mem, size_t _size) :
	mem(mem),
	size(_size),
	prev(nullptr),
	next(nullptr)
{
}

MemoryBlock::~MemoryBlock()
{
	delete prev;
	delete next;
}

Allocator::Allocator() :
	m_parent(nullptr),
	m_memory(nullptr)
{
}
Allocator::Allocator(Allocator* parent, size_t blockSize) :
	m_parent(parent), 
	m_memory(new MemoryBlock(m_parent->allocate(blockSize), blockSize))
{
}
Allocator::~Allocator()
{
	release();
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

void Allocator::request()
{
	// TODO request parent & store new block.
	throw std::bad_alloc();
}

void Allocator::release()
{
	if (m_memory)
	{
		m_parent->deallocate(m_memory->mem, m_memory->size);
		delete m_memory;
	}
}

void* MemoryAllocator::allocate(size_t size, AllocatorFlags flags)
{
	return Memory::alloc(size);
}
void* MemoryAllocator::alignedAllocate(size_t size, size_t alignement, AllocatorFlags flags)
{
	return Memory::alignedAlloc(alignement, size);
}
void MemoryAllocator::deallocate(void* elements, size_t size)
{
	Memory::free(elements);
}
void MemoryAllocator::alignedDeallocate(void* elements, size_t size)
{
	Memory::alignedFree(elements);
}

};