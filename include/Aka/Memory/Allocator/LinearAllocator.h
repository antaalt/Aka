#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Memory/Allocator.h>

namespace aka {

class LinearAllocator : public Allocator
{
public:
	LinearAllocator(const char* name, AllocatorMemoryType memoryType, AllocatorCategory category, Allocator* parent, size_t blockSize);
	virtual ~LinearAllocator();

	void* allocate_internal(size_t size, AllocatorFlags flags = AllocatorFlags::None) override;
	void* alignedAllocate_internal(size_t size, size_t alignement, AllocatorFlags flags = AllocatorFlags::None)  override;
	void deallocate_internal(void* elements, size_t size) override;
	void alignedDeallocate_internal(void* elements, size_t size) override;
private:
	size_t m_offset;
};

};
