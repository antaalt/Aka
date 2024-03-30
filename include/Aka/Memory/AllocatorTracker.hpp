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
private:
	struct AllocatorTrackingData {
		size_t m_allocation = 0;
		size_t m_deallocation = 0;
		size_t m_memoryAllocated = 0;
		size_t m_memoryDeallocated = 0;
		std::map<const void*, AllocationTrackingData> m_allocations;
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