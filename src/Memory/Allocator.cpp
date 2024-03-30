
#include <Aka/Memory/Allocator.h>

#include <Aka/Memory/Memory.h>
#include <Aka/Core/Container/String.h>

namespace aka {

MemoryBlock::MemoryBlock(void* mem, size_t _size) :
	mem(mem),
	size(_size),
	next(nullptr)
{
}

MemoryBlock::~MemoryBlock()
{
	// This does not free allocated memory. allocator need to handle it.
	delete next;
}

Allocator::Allocator(const char* name, AllocatorMemoryType memoryType, AllocatorCategory category) :
	m_name(""),
	m_type(memoryType),
	m_category(category),
	m_parent(nullptr),
	m_memory(nullptr)
{
	String::copy(m_name, 31, name);
	m_name[31] = 0;
}
Allocator::Allocator(const char* name, AllocatorMemoryType memoryType, AllocatorCategory category, Allocator* parent, size_t blockSize) :
	m_name(""),
	m_type(memoryType),
	m_category(category),
	m_parent(parent), 
	m_memory(new MemoryBlock(m_parent->allocate<uint8_t>(blockSize), blockSize))
{
	String::copy(m_name, 32, name);
	m_name[31] = 0;
}
Allocator::~Allocator()
{
	releaseAllMemoryBlocks();
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

MemoryBlock* Allocator::getMemoryBlock()
{
	MemoryBlock* block = m_memory;
	while (block->next != nullptr)
	{
		block = block->next;
	}
	return block;
}

Allocator* Allocator::getParentAllocator()
{
	return m_parent;
}

const char* Allocator::getName() const
{
	return m_name;
}

MemoryBlock* Allocator::requestNewMemoryBlock()
{
	if (m_parent)
	{
		void* mem = m_parent->allocate<uint8_t>(m_memory->size, AllocatorFlags::None);
		MemoryBlock* newBlock = new MemoryBlock(mem, m_memory->size);
		// Put new block at the end of last block
		getMemoryBlock()->next = newBlock;
		return newBlock;
	}
	else 
	{
		throw std::bad_alloc();
		return nullptr;
	}
}

void Allocator::releaseAllMemoryBlocks()
{
	if (m_memory)
	{
		if (m_parent)
		{
			MemoryBlock* block = m_memory;
			while (block)
			{
				m_parent->deallocate<uint8_t>((uint8_t*)block->mem, block->size);
				MemoryBlock* nextBlock = block->next;
				delete block;
				block = nextBlock;
			}
			m_memory = nullptr;
		}
		else
		{
			throw std::bad_alloc();
		}
	}
}

};