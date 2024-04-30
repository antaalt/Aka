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
	return _aligned_realloc(data, size, alignment);
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
Allocator& getAllocator(AllocatorMemoryType memory, AllocatorCategory category)
{
	static DefaultAllocator GlobalMemoryAllocator("GlobalMemoryAllocator", AllocatorMemoryType::Raw, AllocatorCategory::Global);

	static DefaultAllocatorType AllocatorType[EnumCount<AllocatorMemoryType>()][EnumCount<AllocatorCategory>()] = {
		{ // AllocatorMemoryType::Vector
			DefaultAllocatorType("VectorGlobalMemoryAllocator", AllocatorMemoryType::Vector, AllocatorCategory::Global, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("VectorGfxMemoryAllocator", AllocatorMemoryType::Vector, AllocatorCategory::Graphic, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("VectorAudioMemoryAllocator", AllocatorMemoryType::Vector, AllocatorCategory::Audio, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("VectorAssetsMemoryAllocator", AllocatorMemoryType::Vector, AllocatorCategory::Assets, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("VectorComponentMemoryAllocator", AllocatorMemoryType::Vector, AllocatorCategory::Component, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("VectorEditorMemoryAllocator", AllocatorMemoryType::Vector, AllocatorCategory::Editor, &GlobalMemoryAllocator, 1U << 20),
			
		},
		{ // AllocatorMemoryType::Map
			DefaultAllocatorType("MapGlobalMemoryAllocator", AllocatorMemoryType::Map, AllocatorCategory::Global, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("MapGfxMemoryAllocator", AllocatorMemoryType::Map, AllocatorCategory::Graphic, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("MapAudioMemoryAllocator", AllocatorMemoryType::Map, AllocatorCategory::Audio, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("MapAssetsMemoryAllocator", AllocatorMemoryType::Map, AllocatorCategory::Assets, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("MapComponentMemoryAllocator", AllocatorMemoryType::Map, AllocatorCategory::Component, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("MapEditorMemoryAllocator", AllocatorMemoryType::Map, AllocatorCategory::Editor, &GlobalMemoryAllocator, 1U << 20),
		},
		{ // AllocatorMemoryType::Blob
			DefaultAllocatorType("BlobGlobalMemoryAllocator", AllocatorMemoryType::Blob, AllocatorCategory::Global, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("BlobGfxMemoryAllocator", AllocatorMemoryType::Blob, AllocatorCategory::Graphic, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("BlobAudioMemoryAllocator", AllocatorMemoryType::Blob, AllocatorCategory::Audio, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("BlobAssetsMemoryAllocator", AllocatorMemoryType::Blob, AllocatorCategory::Assets, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("BlobComponentMemoryAllocator", AllocatorMemoryType::Blob, AllocatorCategory::Component, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("BlobEditorMemoryAllocator", AllocatorMemoryType::Blob, AllocatorCategory::Editor, &GlobalMemoryAllocator, 1U << 20),
		},
		{ // AllocatorMemoryType::Set
			DefaultAllocatorType("SetGlobalMemoryAllocator", AllocatorMemoryType::Set, AllocatorCategory::Global, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("SetGfxMemoryAllocator", AllocatorMemoryType::Set, AllocatorCategory::Graphic, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("SetAudioMemoryAllocator", AllocatorMemoryType::Set, AllocatorCategory::Audio, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("SetAssetsMemoryAllocator", AllocatorMemoryType::Set, AllocatorCategory::Assets, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("SetComponentMemoryAllocator", AllocatorMemoryType::Set, AllocatorCategory::Component, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("SetEditorMemoryAllocator", AllocatorMemoryType::Set, AllocatorCategory::Editor, &GlobalMemoryAllocator, 1U << 20),
		},
		{ // AllocatorMemoryType::List
			DefaultAllocatorType("ListGlobalMemoryAllocator", AllocatorMemoryType::List, AllocatorCategory::Global, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("ListGfxMemoryAllocator", AllocatorMemoryType::List, AllocatorCategory::Graphic, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("ListAudioMemoryAllocator", AllocatorMemoryType::List, AllocatorCategory::Audio, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("ListAssetsMemoryAllocator", AllocatorMemoryType::List, AllocatorCategory::Assets, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("ListComponentMemoryAllocator", AllocatorMemoryType::List, AllocatorCategory::Component, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("ListEditorMemoryAllocator", AllocatorMemoryType::List, AllocatorCategory::Editor, &GlobalMemoryAllocator, 1U << 20),
		},
		{ // AllocatorMemoryType::String
			DefaultAllocatorType("StringGlobalMemoryAllocator", AllocatorMemoryType::String, AllocatorCategory::Global, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("StringGfxMemoryAllocator", AllocatorMemoryType::String, AllocatorCategory::Graphic, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("StringAudioMemoryAllocator", AllocatorMemoryType::String, AllocatorCategory::Audio, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("StringAssetsMemoryAllocator", AllocatorMemoryType::String, AllocatorCategory::Assets, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("StringComponentMemoryAllocator", AllocatorMemoryType::String, AllocatorCategory::Component, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("StringEditorMemoryAllocator", AllocatorMemoryType::String, AllocatorCategory::Editor, &GlobalMemoryAllocator, 1U << 20),
		},
		{ // AllocatorMemoryType::Pool
			DefaultAllocatorType("PoolGlobalMemoryAllocator", AllocatorMemoryType::Pool, AllocatorCategory::Global, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("PoolGfxMemoryAllocator", AllocatorMemoryType::Pool, AllocatorCategory::Graphic, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("PoolAudioMemoryAllocator", AllocatorMemoryType::Pool, AllocatorCategory::Audio, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("PoolAssetsMemoryAllocator", AllocatorMemoryType::Pool, AllocatorCategory::Assets, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("PoolComponentMemoryAllocator", AllocatorMemoryType::Pool, AllocatorCategory::Component, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("PoolEditorMemoryAllocator", AllocatorMemoryType::Pool, AllocatorCategory::Editor, &GlobalMemoryAllocator, 1U << 20),
		},
		{ // AllocatorMemoryType::Object
			DefaultAllocatorType("ObjectGlobalMemoryAllocator", AllocatorMemoryType::Object, AllocatorCategory::Global, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("ObjectGfxMemoryAllocator", AllocatorMemoryType::Object, AllocatorCategory::Graphic, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("ObjectAudioMemoryAllocator", AllocatorMemoryType::Object, AllocatorCategory::Audio, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("ObjectAssetsMemoryAllocator", AllocatorMemoryType::Object, AllocatorCategory::Assets, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("ObjectComponentMemoryAllocator", AllocatorMemoryType::Object, AllocatorCategory::Component, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("ObjectEditorMemoryAllocator", AllocatorMemoryType::Object, AllocatorCategory::Editor, &GlobalMemoryAllocator, 1U << 20),
		},
		{ // AllocatorMemoryType::Raw
			DefaultAllocatorType("RawGlobalMemoryAllocator", AllocatorMemoryType::Raw, AllocatorCategory::Global, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("RawGfxMemoryAllocator", AllocatorMemoryType::Raw, AllocatorCategory::Graphic, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("RawAudioMemoryAllocator", AllocatorMemoryType::Raw, AllocatorCategory::Audio, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("RawAssetsMemoryAllocator", AllocatorMemoryType::Raw, AllocatorCategory::Assets, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("RawComponentMemoryAllocator", AllocatorMemoryType::Raw, AllocatorCategory::Component, &GlobalMemoryAllocator, 1U << 20),
			DefaultAllocatorType("RawEditorMemoryAllocator", AllocatorMemoryType::Raw, AllocatorCategory::Editor, &GlobalMemoryAllocator, 1U << 20),
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
	return mem::getAllocator(AllocatorMemoryType::Raw, AllocatorCategory::Global).allocate<byte_t>(n, AllocatorFlags::None);
}
void operator delete(void* p, std::size_t) throw()
{
	using namespace aka;
	mem::getAllocator(AllocatorMemoryType::Raw, AllocatorCategory::Global).deallocate(static_cast<byte_t*>(p));
}
void* operator new[](std::size_t n) noexcept(false)
{
	using namespace aka;
	return mem::getAllocator(AllocatorMemoryType::Raw, AllocatorCategory::Global).allocate<byte_t>(n, AllocatorFlags::None);
}
void operator delete[](void* p) throw()
{
	using namespace aka;
	mem::getAllocator(AllocatorMemoryType::Raw, AllocatorCategory::Global).deallocate(static_cast<byte_t*>(p));
}
void operator delete[](void* p, std::size_t) throw()
{
	using namespace aka;
	mem::getAllocator(AllocatorMemoryType::Raw, AllocatorCategory::Global).deallocate(static_cast<byte_t*>(p));
}
#endif