#include <Aka/Memory/Allocator/PoolAllocator.h>

namespace aka {

PoolAllocator::PoolAllocator(void* mem, size_t size, size_t objectSize, size_t objectAlignment) :
	Allocator(mem, size),
	m_objectAlignment(objectAlignment),
	m_objectCount(0),
	m_objectSize(objectSize),
	m_freeList(nullptr)
{
	// We use object memory to store next pointer, so space needs to be bigger
	AKA_ASSERT(objectSize >= sizeof(void*), "Object size needs to be bigger than sizeof(void*)");
	reset();
}

PoolAllocator::~PoolAllocator()
{
}

void* PoolAllocator::allocate(size_t size, size_t alignment)
{
	AKA_ASSERT(size == m_objectSize && alignment == m_objectAlignment, "Invalid size or alignment.");
	if (m_freeList == nullptr) return nullptr;
	void* p = m_freeList;
	m_freeList = (void**)(*m_freeList);
	m_used += size;
	return p;
}

void PoolAllocator::deallocate(void* address, size_t size)
{
	AKA_ASSERT(address > m_mem && (uintptr_t)address < (uintptr_t)m_mem + m_size, "Invalid range");
	AKA_ASSERT(size == m_objectSize, "Invalid size.");
	*((void**)address) = m_freeList;
	m_freeList = (void**)address;
	m_used -= size;
}

void PoolAllocator::reset()
{
	size_t adjustment = Allocator::alignAdjustment((uintptr_t)m_mem, m_objectAlignment);
	m_freeList = (void**)((uintptr_t)m_mem + adjustment);
	m_objectCount = (m_size - adjustment) / m_objectSize;
	void** p = m_freeList;
	for (size_t i = 0; i < m_objectCount - 1; i++)
	{
		*p = (void*)((uintptr_t)p + m_objectSize);
		p = (void**)*p;
	}
	*p = nullptr; // last item.
}

bool PoolAllocator::contiguous() const
{
	return false;
}

};
