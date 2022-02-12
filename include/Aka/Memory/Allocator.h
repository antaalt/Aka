#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Memory/Memory.h>

namespace aka {

class Allocator
{
public:
	Allocator(void* mem, size_t size);
	virtual ~Allocator();

	// Invalidate every pointer created with this allocator and make the whole pool available.
	virtual void reset() = 0;
	// Allocate memory from the allocator
	virtual void* allocate(size_t size, size_t alignement = 0) = 0;
	// Deallocate memory from the allocator
	virtual void deallocate(void* address, size_t size) = 0;
	// Is allocator memory contiguous
	virtual bool contiguous() const = 0;

	// Memory used by this allocator
	const void* mem() const;
	// Size of the memory used
	size_t size() const;
	// Used memory in the allocator
	size_t used() const;

	// Acquire an object from the allocator
	template <typename T, typename ...Args> T* acquire(Args&&... args);
	// Acquire an object array from the allocator
	template <typename T> T* acquires(size_t length);
	// Release an object from the allocator
	template <typename T> void release(T& element);
	// Release an object array from the allocator
	template <typename T> void releases(T* elements);

	// Find next aligned address for given one.
	static uintptr_t align(uintptr_t address, size_t alignment);
	// Find next aligned address adjustment for given one.
	static size_t alignAdjustment(uintptr_t address, size_t alignment);
protected:
	void* m_mem;
	size_t m_size;
	size_t m_used;
};



template <typename T, typename ...Args> T* Allocator::acquire(Args&&... args)
{
	return new (allocate(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
};
template <typename T> T* Allocator::acquires(size_t length)
{
	AKA_ASSERT(length > 0 && contiguous(), "Trying to allocate empty array");
	size_t headerSize = sizeof(size_t) / sizeof(T);// Ensure data is aligned.
	if (sizeof(size_t) % sizeof(T) > 0) headerSize += 1;
	// Allocate with header
	T* allocated = ((T*)allocate(sizeof(T) * (length + headerSize), alignof(T))) + headerSize;
	*(((size_t*)allocated) - 1) = length; // Store length at beginning of array
	// Construct object if needed
	if constexpr (std::is_constructible<T>::value)
		for (size_t i = 0; i < length; i++)
			new (&allocated[i]) T;
	return allocated;
}
template <typename T> void Allocator::release(T& element)
{
	if constexpr (std::is_destructible<T>::value)
		element.~T();
	deallocate(&element, sizeof(T));
}
template <typename T> void Allocator::releases(T* elements)
{
	size_t length = *(((size_t*)elements) - 1);
	if constexpr (std::is_destructible<T>::value)
		for (size_t i = 0; i < length; i++)
			elements[i].~T();
	size_t headerSize = sizeof(size_t) / sizeof(T);
	if (sizeof(size_t) % sizeof(T) > 0)
		headerSize += 1;
	deallocate(elements - headerSize, sizeof(T) * (length + headerSize));
}

};
