#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Memory/Allocator.h>
#include <Aka/Memory/Memory.h>
#include <Aka/OS/Logger.h>

namespace aka {

// Allocator should not be malloc dependent, that is another issue.
class DefaultAllocator : public Allocator
{
public:
	DefaultAllocator(const char* name, AllocatorMemoryType memoryType, AllocatorCategory category, Allocator* parent = nullptr, size_t blockSize = 0);
	virtual ~DefaultAllocator() {}
	void* allocate_internal(size_t size, AllocatorFlags flags = AllocatorFlags::None) override;
	void* alignedAllocate_internal(size_t size, size_t alignement, AllocatorFlags flags = AllocatorFlags::None) override;
	void deallocate_internal(void* elements, size_t size) override;
	void alignedDeallocate_internal(void* elements, size_t size) override;
};

};
