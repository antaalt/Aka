#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Memory/Allocator.h>

namespace aka {

class RingAllocator : public Allocator
{
public:
	RingAllocator(MemoryBlock& block);

	void* allocate_internal(size_t size, AllocatorFlags flags = AllocatorFlags::None) override;
	void* alignedAllocate_internal(size_t size, size_t alignement, AllocatorFlags flags = AllocatorFlags::None)  override;
	void deallocate_internal(void* elements) override;
	void alignedDeallocate_internal(void* elements) override;
private:
	size_t m_offset;
};

};
