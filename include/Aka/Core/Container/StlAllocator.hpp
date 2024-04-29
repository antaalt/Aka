#pragma once

#include <Aka/Memory/Allocator.h>
#include <Aka/Memory/Memory.h>

namespace aka {


template <class T1, AllocatorMemoryType Type = AllocatorMemoryType::Persistent, AllocatorCategory Category = AllocatorCategory::Default> class AkaStlAllocator;

// Specialize for void
template <AllocatorMemoryType Type, AllocatorCategory Category> class AkaStlAllocator<void, Type, Category>
{
public:
	typedef void* pointer;
	typedef const void* const_pointer;
	typedef void value_type;
	template <class U1> struct rebind { typedef AkaStlAllocator<U1> other; };
};

template <class T, AllocatorMemoryType Type, AllocatorCategory Category>
class AkaStlAllocator
{
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	// Description:
	// The rebind member allows a container to construct an allocator for some arbitrary type out of
	// the allocator type provided as a template parameter.
	template <class U> struct rebind { typedef AkaStlAllocator<U, Type, Category> other; };

	// Constructors
	AkaStlAllocator(void) {}
	AkaStlAllocator(const AkaStlAllocator&) {}
	template <class U> AkaStlAllocator(const AkaStlAllocator<U, Type, Category>&) throw() {}

	// Destructor
	~AkaStlAllocator() throw() {}

	// Get the addresses
	pointer address(reference x) const { return &x; }
	const_pointer address(const_reference x) const { return &x; }

	// Allocate storage for s values
	pointer allocate(size_type s, typename AkaStlAllocator<void, Type, Category>::const_pointer hint = 0)
	{
		pointer return_value = aka::mem::getAllocator(Type, Category).allocate<T>(s, aka::AllocatorFlags::None);
		if (return_value == 0)
			throw std::bad_alloc();
		return return_value;
	}
	// Deallocate storage obtained by a call to allocate.
	void deallocate(pointer p, size_type s)
	{
		mem::getAllocator(Type, Category).deallocate(p, s);
	}
	// Return the largest possible storage available through a call to allocate.
	size_type max_size() const throw()
	{
		return std::numeric_limits<size_t>::max() / sizeof(T);
	}
	void construct(pointer ptr)
	{
		::new (reinterpret_cast<void*>(ptr)) T;
	};
	template <class U1> void construct(pointer ptr, const U1& val)
	{
		::new (reinterpret_cast<void*>(ptr)) T(val);
	};
	void construct(pointer ptr, const T& val)
	{
		::new (reinterpret_cast<void*>(ptr)) T(val);
	}
	void destroy(pointer p)
	{
		p->T::~T();
	}
};

// Return true if allocators b and a can be safely interchanged. "Safely interchanged" means that b could be
// used to deallocate storage obtained through a and vice versa.
template <class T1, class T2, AllocatorMemoryType Type, AllocatorCategory Category> bool operator == (const AkaStlAllocator<T1, Type, Category>& a, const AkaStlAllocator<T2, Type, Category>& b)
{
	return true;
};
// Return false if allocators b and a can be safely interchanged. "Safely interchanged" means that b could be
// used to deallocate storage obtained through a and vice versa.
template <class T1, class T2, AllocatorMemoryType Type, AllocatorCategory Category> bool operator != (const AkaStlAllocator<T1, Type, Category>& a, const AkaStlAllocator<T2, Type, Category>& b)
{
	return false;
};

};
