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
	size_t elementSize = 0;
	size_t count = 0;

	template <typename T> static AllocationTrackingData create(size_t count);
};

struct AllocatorTracker
{
public:
	AllocatorTracker();
	~AllocatorTracker();

	template <typename T>
	void allocate(const void* const pointer, size_t count, AllocatorMemoryType type, AllocatorCategory category);
	void allocate(const void* const pointer, AllocatorMemoryType type, AllocatorCategory category, const AllocationTrackingData& data);
	void deallocate(const void* const pointer, AllocatorMemoryType type, AllocatorCategory category);
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
	// custom allocator based on malloc to store allocations without tracking its own allocations.
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
private:
	struct AllocatorTrackingData {
		size_t m_allocation = 0;
		size_t m_deallocation = 0;
		size_t m_memoryAllocated = 0;
		size_t m_memoryDeallocated = 0;
		std::map<const void*, AllocationTrackingData, std::less<const void*>, malloc_allocator<std::pair<const void* const, AllocationTrackingData>>> m_allocations;
	};
	// TODO map with allocator as pointer key
	AllocatorTrackingData m_data[EnumCount<AllocatorMemoryType>()][EnumCount<AllocatorCategory>()];
};

AllocatorTracker& getAllocatorTracker();

template <typename T>
static AllocationTrackingData AllocationTrackingData::create(size_t count) {
	AllocationTrackingData data;
	data.info = &typeid(T);
	data.elementSize = sizeof(T);
	data.count = count;
	return data;
}

template <typename T>
void AllocatorTracker::allocate(const void* const pointer, size_t count, AllocatorMemoryType type, AllocatorCategory category)
{
	allocate(pointer, type, category, AllocationTrackingData::create<T>(count));
}
#endif

};