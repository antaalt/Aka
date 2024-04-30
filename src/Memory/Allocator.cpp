
#include <Aka/Memory/Allocator.h>

#include <Aka/Memory/Memory.h>
#include <Aka/Core/Container/String.h>

namespace aka {


const char* toString(AllocatorMemoryType _type)
{
	switch (_type)
	{
	case aka::AllocatorMemoryType::Vector: return "Vector";
	case aka::AllocatorMemoryType::Map: return "Map";
	case aka::AllocatorMemoryType::Set: return "Set";
	case aka::AllocatorMemoryType::Blob: return "Blob";
	case aka::AllocatorMemoryType::List: return "List";
	case aka::AllocatorMemoryType::String: return "String";
	case aka::AllocatorMemoryType::Pool: return "Pool";
	case aka::AllocatorMemoryType::Object: return "Object";
	case aka::AllocatorMemoryType::Raw: return "Raw";
	default: return "Unknown";
	}
}
const char* toString(AllocatorCategory _category)
{
	switch (_category)
	{
	case aka::AllocatorCategory::Global: return "Global";
	case aka::AllocatorCategory::Graphic: return "Graphic";
	case aka::AllocatorCategory::Audio: return "Audio";
	case aka::AllocatorCategory::Assets: return "Assets";
	case aka::AllocatorCategory::Component: return "Component";
	case aka::AllocatorCategory::Editor: return "Editor";
	default: return "Unkown";
	}
}

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
				m_parent->deallocate((uint8_t*)block->mem);
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