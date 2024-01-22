#include <Aka/Memory/Memory.h>

namespace aka {

Memory::Memory(size_t size) :
	m_size(size), 
	m_data(Memory::alloc(size)) 
{
}

Memory::Memory(const Memory& memory) :
	Memory(memory.size())
{
	Memory::copy(m_data, memory.raw(), memory.size());
}

Memory& Memory::operator=(const Memory& memory)
{
	if (m_size != memory.size())
	{
		Memory::free(m_data);
		m_data = Memory::alloc(memory.size());
		m_size = memory.size();
	}
	Memory::copy(m_data, memory.raw(), memory.size());
	return *this;
}

Memory::Memory(Memory&& memory) :
	m_size(0),
	m_data(nullptr)
{
	std::swap(m_size, memory.m_size);
	std::swap(m_data, memory.m_data);
}

Memory& Memory::operator=(Memory&& memory)
{
	std::swap(m_size, memory.m_size);
	std::swap(m_data, memory.m_data);
	return *this;
}

Memory::~Memory()
{ 
	Memory::free(m_data);
}

size_t Memory::size() const
{ 
	return m_size; 
}

void* Memory::raw() const
{ 
	return m_data; 
}

void* Memory::alloc(size_t size)
{
	// TODO request os memory with brk / mmap
	return ::malloc(size);
}

void* Memory::alignedAlloc(size_t alignment, size_t size)
{
#if defined(AKA_PLATFORM_WINDOWS)
	return _aligned_malloc(size, alignment);
#else // POSIX
	return aligned_alloc(alignment, size);
#endif
}

void Memory::free(void* data)
{
	::free(data); 
}

void Memory::alignedFree(void* data)
{
#if defined(AKA_PLATFORM_WINDOWS)
	_aligned_free(data);
#else // POSIX
	free(data);
#endif
}
void* Memory::realloc(void* data, size_t size)
{
	return ::realloc(data, size);
}

void* Memory::alignedRealloc(void* data, size_t alignment, size_t size)
{
#if defined(AKA_PLATFORM_WINDOWS)
	return _aligned_realloc(data, alignment, size);
#else // POSIX
	free(data); // No aligned realloc for POSIX
	return aligned_alloc(alignment, size);
#endif
}

void* Memory::copy(void* dst, const void* src, size_t size)
{
	return ::memcpy(dst, src, size);
}

void Memory::set(void* memory, uint8_t data, size_t size)
{
	::memset(memory, data, size);
}

int32_t Memory::compare(const void* lhs, const void* rhs, size_t size)
{
	return ::memcmp(lhs, rhs, size);
}

void Memory::zero(void* memory, size_t size)
{
	Memory::set(memory, 0, size);
}

bool Memory::operator==(const Memory& rhs)
{
	if (m_size != rhs.m_size)
		return false;
	return Memory::compare(m_data, rhs.m_data, m_size) == 0;
}

bool Memory::operator!=(const Memory& rhs)
{
	if (m_size == rhs.m_size)
		return false;
	return Memory::compare(m_data, rhs.m_data, m_size) != 0;
}

namespace mem {

// There should be some memory manager running everyframe & updating all blocks.
Allocator& getAllocator(AllocatorMemoryType memory = AllocatorMemoryType::Persistent, AllocatorCategory category = AllocatorCategory::Default)
{
	static MemoryAllocator GfxMemoryAllocator;
	static MemoryAllocator AudioMemoryAllocator;
	static MemoryAllocator DefaultMemoryAllocator;

	static DefaultAllocatorType GfxPersistentAllocator(&GfxMemoryAllocator, 1 << 16);
	static DefaultAllocatorType DefaultPersistentAllocator(&DefaultMemoryAllocator, 1LL << 31);
	static DefaultAllocatorType StringPersistentAllocator(&DefaultMemoryAllocator, 1 << 16); // 512 MB

	static TemporaryAllocatorType GfxTemporaryAllocator(&GfxMemoryAllocator, 1 << 16);
	static TemporaryAllocatorType DefaultTemporaryAllocator(&DefaultMemoryAllocator, 1 << 16); // 512 MB
	static TemporaryAllocatorType StringTemporaryAllocator(&DefaultMemoryAllocator, 1 << 16);
	switch (memory)
	{
	case aka::mem::AllocatorMemoryType::Temporary: {
		switch (category)
		{
		default:
		case AllocatorCategory::Default:
			return DefaultMemoryAllocator;// DefaultTemporaryAllocator;
		case AllocatorCategory::Graphic:
			return DefaultMemoryAllocator;//GfxTemporaryAllocator;
		case AllocatorCategory::String:
			return DefaultMemoryAllocator;//StringTemporaryAllocator;
		}
	}
	default:
	case aka::mem::AllocatorMemoryType::Persistent: {
		switch (category)
		{
		default:
		case AllocatorCategory::Default:
			return DefaultMemoryAllocator;//DefaultPersistentAllocator;
		case AllocatorCategory::Graphic:
			return DefaultMemoryAllocator;//GfxPersistentAllocator;
		case AllocatorCategory::String:
			return DefaultMemoryAllocator;//StringPersistentAllocator;
		}
	}
	}
}

};
};