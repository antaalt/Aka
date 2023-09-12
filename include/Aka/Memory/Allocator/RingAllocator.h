#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Memory/Allocator.h>

namespace aka {

class RingAllocator : public Allocator
{
public:
	RingAllocator(MemoryBlock& block);

	void* allocate(size_t size, AllocatorFlags flags = AllocatorFlags::None) override;
	void* alignedAllocate(size_t size, size_t alignement, AllocatorFlags flags = AllocatorFlags::None)  override;
	void deallocate(void* elements, size_t size) override;
	void alignedDeallocate(void* elements, size_t size) override;
private:
	size_t m_offset;
};

};
