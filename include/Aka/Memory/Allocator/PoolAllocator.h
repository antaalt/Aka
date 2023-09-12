#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Memory/Allocator.h>

namespace aka {

/*class PoolAllocator final : public Allocator
{
public:
	PoolAllocator(void* mem, size_t size, size_t objectSize, size_t objectAlignment);
	~PoolAllocator();

	void* allocate(size_t size, size_t alignment = 0) override;
	void deallocate(void* address, size_t size) override;
	void reset() override;
	bool contiguous() const override;
private:
	size_t m_objectAlignment;
	size_t m_objectCount;
	size_t m_objectSize;
	void** m_freeList;
};*/

};
