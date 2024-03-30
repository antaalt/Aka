#include <Aka/Memory/AllocatorTracker.hpp>

#include <Aka/Core/Config.h>

#include <iostream>
#include <vector>

namespace aka {

#if defined(AKA_TRACK_MEMORY_ALLOCATIONS)

const char* toString(AllocatorMemoryType type)
{
	switch (type)
	{
	case AllocatorMemoryType::Temporary:
		return "Temporary";
	case AllocatorMemoryType::Persistent:
		return "Persistent";
	default:
		return "Unknown";
	}
}
const char* toString(AllocatorCategory category)
{
	switch (category)
	{
	case AllocatorCategory::Default:
		return "Default";
	case AllocatorCategory::Graphic:
		return "Graphic";
	case AllocatorCategory::String:
		return "String";
	case AllocatorCategory::Vector:
		return "Vector";
	case AllocatorCategory::Pool:
		return "Pool";
	case AllocatorCategory::List:
		return "List";
	case AllocatorCategory::Component:
		return "Component";
	default:
		return "Unknown";
	}
}

AllocatorTracker::AllocatorTracker() 
{
}
AllocatorTracker::~AllocatorTracker()
{
	size_t totalLeak = 0;
	for (AllocatorMemoryType type : EnumRange<AllocatorMemoryType>())
	{
		std::cout << "----- AllocatorMemoryType::" << toString(type) << " ------------" << std::endl;
		for (AllocatorCategory category : EnumRange<AllocatorCategory>())
		{
			size_t totalLeakCategory = 0;
			std::cout << "	--- AllocatorCategory::" << toString(category) << " ------------" << std::endl;
			const AllocatorTrackingData& data = m_data[EnumToIndex(type)][EnumToIndex(category)];
			std::cout << "		" << data.m_allocation << " allocations for " << data.m_memoryAllocated << " bytes" << std::endl;
			std::cout << "		" << data.m_deallocation << " deallocations for " << data.m_memoryDeallocated << " bytes" << std::endl;
			std::cout << "		" << data.m_allocation - data.m_deallocation << " allocation leak detected for " << data.m_memoryAllocated - data.m_memoryDeallocated << " bytes" << std::endl;
			if (data.m_allocations.size() > 0)
				std::cout << "		--" << std::endl;
			for (const auto& alloc : data.m_allocations)
			{
				const AllocationTrackingData& data = alloc.second;
				std::cout << "		Leak : " << data.info->name() << " (size(" << data.elementSize << " bytes) count(" << data.count << ")" << std::endl;
				totalLeakCategory += data.elementSize * data.count;
			}
			std::cout << "		--" << std::endl;
			std::cout << "		Total leaks category detected : " << totalLeakCategory << " bytes" << std::endl;
			totalLeak += totalLeakCategory;
		}
	}
	std::cout << "Total leaks detected : " << totalLeak << " bytes" << std::endl;
}

void AllocatorTracker::allocate(const void* const pointer, AllocatorMemoryType type, AllocatorCategory category, const AllocationTrackingData& data)
{
	if (pointer == nullptr)
		return;
	//std::cout << "Allocation for " << data.info->name() << " of " << data.count << " elements of size " << data.elementSize << std::endl;
	AKA_ASSERT(EnumIsInRange(type), "Invalid allocator type");
	AKA_ASSERT(EnumIsInRange(category), "Invalid allocator category");
	AKA_ASSERT(data.elementSize > 0, "Invalid allocation");
	AllocatorTrackingData& tracking = m_data[EnumToIndex(type)][EnumToIndex(category)];
	tracking.m_allocation += 1;
	tracking.m_memoryAllocated += data.elementSize * data.count;
	tracking.m_allocations.insert(std::make_pair(pointer, data));
}
void AllocatorTracker::deallocate(const void* const pointer, AllocatorMemoryType type, AllocatorCategory category)
{
	if (pointer == nullptr)
		return;
	AllocatorTrackingData& tracking = m_data[EnumToIndex(type)][EnumToIndex(category)];
	auto it = tracking.m_allocations.find(pointer);
	AKA_ASSERT(it != tracking.m_allocations.end(), "Missing allocation, or double deallocation");
	AKA_ASSERT(tracking.m_memoryAllocated > tracking.m_memoryDeallocated, "Invalid deallocation");
	AKA_ASSERT(tracking.m_allocation > tracking.m_deallocation, "Invalid deallocation");
	const AllocationTrackingData& data = it->second;
	//std::cout << "Deallocation for " << data.info->name() << " of " << data.count << " elements of size " << data.elementSize << std::endl;
	tracking.m_memoryDeallocated += data.elementSize * data.count;
	tracking.m_deallocation += 1;

	tracking.m_allocations.erase(pointer);
}
AllocatorTracker& getAllocatorTracker()
{ 
	static AllocatorTracker s_instance;
	return s_instance; 
}

#endif

};