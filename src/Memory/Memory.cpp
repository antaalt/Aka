#include <Aka/Memory/Memory.h>

#include <Aka/Memory/Allocator/LinearAllocator.h>
#include <Aka/Memory/Allocator/RingAllocator.h>
#include <Aka/Memory/Allocator/DebugAllocator.h>

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

void* Memory::allocAlligned(size_t alignment, size_t size)
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

void Memory::freeAligned(void* data)
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

void* Memory::reallocAligned(void* data, size_t alignment, size_t size)
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

#define AKA_TRACK_MEMORY 1

// Until we create a better allocator, use memory
#if defined(AKA_TRACK_MEMORY)
using DefaultAllocatorType = DebugAllocator<MemoryAllocator>;
using TemporaryAllocatorType = DebugAllocator<MemoryAllocator>;
#else
using DefaultAllocatorType = MemoryAllocator;
using TemporaryAllocatorType = MemoryAllocator;
#endif

// There should be some memory manager running everyframe & updating all blocks.
Allocator& getAllocator(AllocatorMemoryType memory = AllocatorMemoryType::Persistent, AllocatorCategory category = AllocatorCategory::Default)
{
	static MemoryAllocator GfxMemoryAllocator("GfxMemoryAllocator");
	static MemoryAllocator AudioMemoryAllocator("AudioMemoryAllocator");
	static MemoryAllocator DefaultMemoryAllocator("DefaultMemoryAllocator");

	static DefaultAllocatorType GfxPersistentAllocator("GfxPersistentAllocator", &GfxMemoryAllocator, 1 << 16);
	static DefaultAllocatorType DefaultPersistentAllocator("DefaultPersistentAllocator", &DefaultMemoryAllocator, 1LL << 31);
	static DefaultAllocatorType StringPersistentAllocator("StringPersistentAllocator", &DefaultMemoryAllocator, 1 << 16); // 512 MB

	static TemporaryAllocatorType GfxTemporaryAllocator("GfxTemporaryAllocator", &GfxMemoryAllocator, 1 << 16);
	static TemporaryAllocatorType DefaultTemporaryAllocator("DefaultTemporaryAllocator", &DefaultMemoryAllocator, 1 << 16); // 512 MB
	static TemporaryAllocatorType StringTemporaryAllocator("StringTemporaryAllocator",  & DefaultMemoryAllocator, 1 << 16);
	switch (memory)
	{
	case aka::mem::AllocatorMemoryType::Temporary: {
		switch (category)
		{
		default:
		case AllocatorCategory::Default:
			return DefaultTemporaryAllocator;
		case AllocatorCategory::Graphic:
			return GfxTemporaryAllocator;
		case AllocatorCategory::String:
			return StringTemporaryAllocator;
		}
	}
	default:
	case aka::mem::AllocatorMemoryType::Persistent: {
		switch (category)
		{
		default:
		case AllocatorCategory::Default:
			return DefaultPersistentAllocator;
		case AllocatorCategory::Graphic:
			return GfxPersistentAllocator;
		case AllocatorCategory::String:
			return StringPersistentAllocator;
		}
	}
	}
}

}; // namespace mem
}; // namespace aka

#if defined(AKA_TRACK_MEMORY)
void* operator new(std::size_t n) throw(std::bad_alloc)
{
	using namespace aka;
	return mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default).allocate(n, AllocatorFlags::None);
}
void operator delete(void* p, std::size_t n) throw()
{
	using namespace aka;
	mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default).deallocate(p, n);
}
void* operator new[](std::size_t n) throw(std::bad_alloc)
{
	using namespace aka;
	return mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default).allocate(n, AllocatorFlags::None);
}
void operator delete[](void* p, std::size_t n) throw()
{
	using namespace aka;
	mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default).deallocate(p, n);
}
#endif