#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Memory/Allocator.h>

namespace aka {

class LinearAllocator : public Allocator
{
public:
	LinearAllocator(const char* name, Allocator* parent, size_t blockSize);
	virtual ~LinearAllocator();

	void* allocate(size_t size, AllocatorFlags flags = AllocatorFlags::None) override;
	void* alignedAllocate(size_t size, size_t alignement, AllocatorFlags flags = AllocatorFlags::None)  override;
	void deallocate(void* elements, size_t size) override;
	void alignedDeallocate(void* elements, size_t size) override;
private:
	size_t m_offset;
};

};
