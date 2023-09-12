#pragma once

#include <Aka/Core/Debug.h>
#include <Aka/Memory/Allocator.h>

namespace aka {

/*class StackAllocator final : public Allocator
{
public:
	StackAllocator(void* mem, size_t size) :
		Allocator(mem, size),
		m_current(mem)
	{
	}
	~StackAllocator()
	{
	}
	void* allocate(size_t size, size_t alignement, size_t offset) override
	{
		uintptr_t current = (uintptr_t)m_mem + m_used;
		size_t headerSize = sizeof(uint8_t);
		size_t adjustment = Allocator::alignAdjustment(current + headerSize, alignement);
		if (m_used + headerSize + adjustment + size > m_size)
			return nullptr; // Not enough space in allocator
		uintptr_t aligned = current + headerSize + adjustment;
		m_used += size + headerSize + adjustment;
		uint8_t* header = reinterpret_cast<uint8_t*>(aligned - headerSize);
		*header = (uint8_t)adjustment;
		m_current = (void*)aligned;
		return (void*)aligned;
	}
	void deallocate(void* address, size_t size) override
	{
		AKA_ASSERT(address > m_mem && (uintptr_t)address < (uintptr_t)m_mem + m_size, "Invalid range");
		//AKA_ASSERT(address + size - mem, "Invalid deallocation for stack.");
		// TODO check we are deallocating the whole top of stack with size.
		uint8_t* header = reinterpret_cast<uint8_t*>(address - sizeof(uint8_t));
		*header = (uint8_t)adjustment;
	}
	void reset() override
	{
		m_used = 0;
	}
	bool contiguous() const override
	{
		return true;
	}
private:
	void* m_current;
};*/

};
