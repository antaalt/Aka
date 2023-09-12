#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Memory/Allocator.h>
#include <Aka/OS/Logger.h>

namespace aka {

template <typename T>
class DebugAllocator : public T
{
public:
	static_assert(std::is_base_of<Allocator, T>::value, "Invalid type");
	DebugAllocator(const MemoryBlock& block);

	void* allocate(size_t size, AllocatorFlags flags = AllocatorFlags::None) override;
	void* alignedAllocate(size_t size, size_t alignement, AllocatorFlags flags = AllocatorFlags::None) override;
	void deallocate(void* elements, size_t size) override;
	void alignedDeallocate(void* elements, size_t size) override;
private:
	size_t m_allocated;
	size_t m_freed;
};

template <typename T>
DebugAllocator<T>::DebugAllocator(const MemoryBlock& block) :
	T(block),
	m_allocated(0),
	m_freed(0)
{
}

template <typename T>
void* DebugAllocator<T>::allocate(size_t size, AllocatorFlags flags)
{
	m_allocated += size;
	std::cerr << std::this_thread::get_id() << " | ";
	std::cerr << "Allocating " << size << " bytes";
	std::cerr << "(allocated: " << m_allocated << "/" << m_memory.size << " bytes, freed " << m_freed << " bytes)" << std::endl;
	return T::allocate(size, flags);
}

template <typename T>
void* DebugAllocator<T>::alignedAllocate(size_t size, size_t alignement, AllocatorFlags flags)
{
	m_allocated += size; // TODO add aligmenent
	std::cerr << std::this_thread::get_id() << " | ";
	std::cerr << "Allocating aligned " << size << " bytes";
	std::cerr << "(allocated: " << m_allocated << "/" << m_memory.size << " bytes, freed " << m_freed << " bytes)" << std::endl;
	return T::alignedAllocate(size, alignement, flags);
}

template <typename T>
void DebugAllocator<T>::deallocate(void* address, size_t size)
{
	m_freed += size;
	std::cerr << std::this_thread::get_id() << " | ";
	std::cerr << "Deallocating " << size << " bytes";
	std::cerr << "(allocated: " << m_allocated << "/" << m_memory.size << " bytes, freed " << m_freed << " bytes)" << std::endl;
	return T::deallocate(address, size);
}

template <typename T>
void DebugAllocator<T>::alignedDeallocate(void* address, size_t size)
{
	m_freed += size; // TODO add aligmenent
	std::cerr << std::this_thread::get_id() << " | ";
	std::cerr << "Deallocating aligned " << size << " bytes";
	std::cerr << "(allocated: " << m_allocated << "/" << m_memory.size << " bytes, freed " << m_freed << " bytes)" << std::endl;
	return T::alignedDeallocate(address, size);
}

};
