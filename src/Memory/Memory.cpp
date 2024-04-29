#include <Aka/Memory/Memory.h>

#include <Aka/Memory/Allocator/LinearAllocator.h>
#include <Aka/Memory/Allocator/RingAllocator.h>
#include <Aka/Memory/Allocator/DefaultAllocator.h>
#include <Aka/Core/Enum.h>

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
	// https://learn.microsoft.com/fr-fr/windows/win32/memory/comparing-memory-allocation-methods
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

// Until we create a better allocator, use malloc
using DefaultAllocatorType = DefaultAllocator;

// There should be some memory manager running everyframe & updating all blocks.
Allocator& getAllocator(AllocatorMemoryType memory = AllocatorMemoryType::Persistent, AllocatorCategory category = AllocatorCategory::Default)
{
	static DefaultAllocator GlobalMemoryAllocator("GlobalMemoryAllocator", AllocatorMemoryType::Persistent, AllocatorCategory::Default);

	static DefaultAllocatorType AllocatorType[EnumCount<AllocatorMemoryType>()][EnumCount<AllocatorCategory>()] = {
		{ // AllocatorMemoryType::Temporary
			DefaultAllocatorType("TemporaryDefaultMemoryAllocator", AllocatorMemoryType::Temporary, AllocatorCategory::Default, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Default
			DefaultAllocatorType("TemporaryGfxMemoryAllocator", AllocatorMemoryType::Temporary, AllocatorCategory::Graphic, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Graphic
			DefaultAllocatorType("TemporaryAudioMemoryAllocator", AllocatorMemoryType::Temporary, AllocatorCategory::Audio, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Audio
			DefaultAllocatorType("TemporaryStringMemoryAllocator", AllocatorMemoryType::Temporary, AllocatorCategory::String, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::String
			DefaultAllocatorType("TemporaryVectorMemoryAllocator", AllocatorMemoryType::Temporary, AllocatorCategory::Vector, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Vector
			DefaultAllocatorType("TemporaryMapMemoryAllocator", AllocatorMemoryType::Temporary, AllocatorCategory::Map, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Map
			DefaultAllocatorType("TemporaryPoolMemoryAllocator", AllocatorMemoryType::Temporary, AllocatorCategory::Pool, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Pool
			DefaultAllocatorType("TemporaryListMemoryAllocator", AllocatorMemoryType::Temporary, AllocatorCategory::List, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::List
			DefaultAllocatorType("TemporaryComponentMemoryAllocator", AllocatorMemoryType::Temporary, AllocatorCategory::Component, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Component
			DefaultAllocatorType("TemporaryArchiveMemoryAllocator", AllocatorMemoryType::Temporary, AllocatorCategory::Archive, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Archive
			DefaultAllocatorType("TemporaryEditorMemoryAllocator", AllocatorMemoryType::Temporary, AllocatorCategory::Editor, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Editor
		},
		{ // AllocatorMemoryType::Persistent
			DefaultAllocatorType("PersistentDefaultMemoryAllocator", AllocatorMemoryType::Persistent, AllocatorCategory::Default, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Default
			DefaultAllocatorType("PersistentGfxMemoryAllocator", AllocatorMemoryType::Persistent, AllocatorCategory::Graphic, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Graphic
			DefaultAllocatorType("PersistentAudioMemoryAllocator", AllocatorMemoryType::Persistent, AllocatorCategory::Audio, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Audio
			DefaultAllocatorType("PersistentStringMemoryAllocator", AllocatorMemoryType::Persistent, AllocatorCategory::String, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::String
			DefaultAllocatorType("PersistentVectorMemoryAllocator", AllocatorMemoryType::Persistent, AllocatorCategory::Vector, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Vector
			DefaultAllocatorType("PersistentMapMemoryAllocator", AllocatorMemoryType::Persistent, AllocatorCategory::Map, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Map
			DefaultAllocatorType("PersistentPoolMemoryAllocator", AllocatorMemoryType::Persistent, AllocatorCategory::Pool, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Pool
			DefaultAllocatorType("PersistentListMemoryAllocator", AllocatorMemoryType::Persistent, AllocatorCategory::List, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::List
			DefaultAllocatorType("PersistentComponentMemoryAllocator", AllocatorMemoryType::Persistent, AllocatorCategory::Component, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Component
			DefaultAllocatorType("PersistentArchiveMemoryAllocator", AllocatorMemoryType::Persistent, AllocatorCategory::Archive, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Archive
			DefaultAllocatorType("PersistentEditorMemoryAllocator", AllocatorMemoryType::Persistent, AllocatorCategory::Editor, &GlobalMemoryAllocator, 1U << 20), // AllocatorCategory::Editor
		},
	};
	return AllocatorType[EnumToIndex(memory)][EnumToIndex(category)];
}

}; // namespace mem
}; // namespace aka

#if defined(AKA_TRACK_MEMORY_ALLOCATIONS)

using byte_t = uint8_t;
void* operator new(std::size_t n) noexcept(false)
{
	using namespace aka;
	return mem::getAllocator(AllocatorMemoryType::Persistent, AllocatorCategory::Default).allocate<byte_t>(n, AllocatorFlags::None);
}
void operator delete(void* p, std::size_t) throw()
{
	using namespace aka;
	mem::getAllocator(AllocatorMemoryType::Persistent, AllocatorCategory::Default).deallocate(static_cast<byte_t*>(p));
}
void* operator new[](std::size_t n) noexcept(false)
{
	using namespace aka;
	return mem::getAllocator(AllocatorMemoryType::Persistent, AllocatorCategory::Default).allocate<byte_t>(n, AllocatorFlags::None);
}
void operator delete[](void* p) throw()
{
	using namespace aka;
	mem::getAllocator(AllocatorMemoryType::Persistent, AllocatorCategory::Default).deallocate(static_cast<byte_t*>(p));
}
void operator delete[](void* p, std::size_t) throw()
{
	using namespace aka;
	mem::getAllocator(AllocatorMemoryType::Persistent, AllocatorCategory::Default).deallocate(static_cast<byte_t*>(p));
}
#endif