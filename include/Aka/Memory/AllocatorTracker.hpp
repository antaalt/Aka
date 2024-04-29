#pragma once

#include <utility>
#include <map>
#include <typeinfo>

#include <Aka/Memory/AllocatorTypes.hpp>
#include <Aka/Core/Enum.h>

namespace aka {

#define AKA_TRACK_MEMORY_ALLOCATIONS

#if defined(AKA_TRACK_MEMORY_ALLOCATIONS)

struct AllocationTrackingData
{
	// typeid pointer live till the end of program
	const std::type_info* info = nullptr;
	size_t alignment = 0;
	size_t elementSize = 0;
	size_t count = 0;

	template <typename T> static AllocationTrackingData create(size_t count, size_t alignment = 0);
};

struct AllocatorTrackingData 
{
	// custom allocator based on malloc to store allocations without tracking its own allocations.
private:
	template <class T>
	struct malloc_allocator {
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;

		template <class U> struct rebind { typedef malloc_allocator<U> other; };
		malloc_allocator() throw() {}
		malloc_allocator(const malloc_allocator&) throw() {}

		template <class U> malloc_allocator(const malloc_allocator<U>&) throw() {}

		~malloc_allocator() throw() {}

		pointer address(reference x) const { return &x; }
		const_pointer address(const_reference x) const { return &x; }

		pointer allocate(size_type s, void const* = 0) {
			if (0 == s)
				return NULL;
			pointer temp = (pointer)malloc(s * sizeof(T));
			if (temp == NULL)
				throw std::bad_alloc();
			return temp;
		}

		void deallocate(pointer p, size_type) {
			free(p);
		}

		size_type max_size() const throw() {
			return std::numeric_limits<size_t>::max() / sizeof(T);
		}

		void construct(pointer p, const T& val) {
			new((void*)p) T(val);
		}

		void destroy(pointer p) {
			p->~T();
		}
	};
	using AllocationMap = std::map<const void*, AllocationTrackingData, std::less<const void*>, malloc_allocator<std::pair<const void* const, AllocationTrackingData>>>;
public:
	size_t m_allocation = 0;
	size_t m_deallocation = 0;
	size_t m_memoryAllocated = 0;
	size_t m_memoryDeallocated = 0;
	AllocationMap m_allocations;

	AllocationMap::iterator begin() { return m_allocations.begin(); }
	AllocationMap::iterator end() { return m_allocations.end(); }
	AllocationMap::const_iterator begin() const { return m_allocations.begin(); }
	AllocationMap::const_iterator end() const { return m_allocations.end(); }
};

struct AllocatorTracker
{
public:
	AllocatorTracker();
	~AllocatorTracker();

	template <typename T> void allocate(const void* const pointer, size_t count, AllocatorMemoryType type, AllocatorCategory category);
	template <typename T> void alignedAllocate(const void* const pointer, size_t count, size_t alignment, AllocatorMemoryType type, AllocatorCategory category);
	void allocate(const void* const pointer, AllocatorMemoryType type, AllocatorCategory category, const AllocationTrackingData& data);
	void deallocate(const void* const pointer, AllocatorMemoryType type, AllocatorCategory category);

	const AllocatorTrackingData& get(AllocatorMemoryType _type, AllocatorCategory _category) { return m_data[EnumToIndex(_type)][EnumToIndex(_category)]; }
private:
	// Override class new & delete to not use global ones.
	void* operator new(std::size_t n) noexcept(false)
	{
		return malloc(sizeof(n));
	}
	void operator delete(void* p, std::size_t n) throw()
	{
		free(p);
	}
	void* operator new[](std::size_t n) noexcept(false)
	{
		return malloc(sizeof(n));
	}
	void operator delete[](void* p, std::size_t n) throw()
	{
		free(p);
	}
	
private:
	// TODO map with allocator as pointer key
	AllocatorTrackingData m_data[EnumCount<AllocatorMemoryType>()][EnumCount<AllocatorCategory>()];
};

AllocatorTracker& getAllocatorTracker();

template <typename T>
static AllocationTrackingData AllocationTrackingData::create(size_t count, size_t alignment) {
	AllocationTrackingData data;
	data.info = &typeid(T);
	data.elementSize = sizeof(T);
	data.alignment = alignment;
	data.count = count;
	return data;
}

template <typename T>
void AllocatorTracker::allocate(const void* const pointer, size_t count, AllocatorMemoryType type, AllocatorCategory category)
{
	allocate(pointer, type, category, AllocationTrackingData::create<T>(count));
}

template <typename T>
void AllocatorTracker::alignedAllocate(const void* const pointer, size_t count, size_t alignment, AllocatorMemoryType type, AllocatorCategory category)
{
	allocate(pointer, type, category, AllocationTrackingData::create<T>(count, alignment));
}
#endif

};