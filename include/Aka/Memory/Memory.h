#pragma once

#include <Aka/Core/Config.h>

#include <Aka/Memory/Allocator.h>

namespace aka {

using byte_t = uint8_t;

class Memory
{
public:
	Memory(size_t size);
	Memory(const Memory& memory);
	Memory& operator=(const Memory& memory);
	Memory(Memory&& memory);
	Memory& operator=(Memory&& memory);
	~Memory();

public:
	// Get the size of the memory block
	size_t size() const;
	// Get the raw pointer to it
	void* raw() const;

public:
	// Allocate memory for given size
	static void* alloc(size_t size);
	// Allocate aligned memory for given size
	static void* allocAlligned(size_t alignment, size_t size);
	// Deallocate memory
	static void (free)(void* data);
	// Deallocate aligned memory
	static void (freeAligned)(void* data);
	// Reallocate memory
	static void* (realloc)(void* data, size_t size);
	// Reallocate aligned memory
	static void* (reallocAligned)(void* data, size_t alignment, size_t size);
	// Copy memory 
	static void* copy(void* dst, const void* src, size_t count);
	// Set memory
	static void set(void* memory, uint8_t data, size_t size);
	// Compare memory
	static int32_t compare(const void* lhs, const void* rhs, size_t size);
	// Set memory to zero
	static void zero(void* memory, size_t size);
	// Compare memories
	bool operator==(const Memory& rhs);
	// Compare memories
	bool operator!=(const Memory& rhs);
private:
	size_t m_size;
	void* m_data;
};

// Default allocator for whole app.
namespace mem {

Allocator& getAllocator(AllocatorMemoryType memory, AllocatorCategory category);

// Override default new for memory tracking.
template <typename T, typename ...Args> T*   akaNew(AllocatorMemoryType type, AllocatorCategory category, Args ...args);
template <typename T>					void akaDelete(const T* pointer);
template <typename T>					void akaSafeDelete(const T* const & pointer);
template <typename T>					T*   akaNewArray(size_t count, AllocatorMemoryType type, AllocatorCategory category);
template <typename T>					void akaDeleteArray(const T* const & pointer);

struct AkaNewHead // u16
{
	uint8_t magicNumber : 8;
	AllocatorMemoryType type : EnumBitCount<AllocatorMemoryType>();
	AllocatorCategory category : EnumBitCount<AllocatorCategory>();
	uint8_t padding : 16 - 8 - EnumBitCount<AllocatorMemoryType>() - EnumBitCount<AllocatorCategory>();
	static_assert(EnumBitCount<AllocatorMemoryType>() + EnumBitCount<AllocatorCategory>() + 8 < 16);
};

struct AkaNewArrayHead // u32
{
	using Type = uint32_t;
	uint8_t magicNumber : 8;
	AllocatorMemoryType type : EnumBitCount<AllocatorMemoryType>();
	AllocatorCategory category : EnumBitCount<AllocatorCategory>();
	size_t count : 32 - 8 - EnumBitCount<AllocatorMemoryType>() - EnumBitCount<AllocatorCategory>();
	static_assert(EnumBitCount<AllocatorMemoryType>() + EnumBitCount<AllocatorCategory>() + 8 < 32);
};

template <typename T, typename ...Args>
T* akaNew(AllocatorMemoryType type, AllocatorCategory category, Args ...args)
{
#if defined(AKA_TRACK_MEMORY_ALLOCATIONS)
	T* data = aka::mem::getAllocator(type, category).allocate<T, AkaNewHead>(1, aka::AllocatorFlags::None);
	// Store metadata in a header.
	AKA_ASSERT(EnumIsInRange(type), "Type invalid");
	AKA_ASSERT(EnumIsInRange(category), "Category invalid");
	AkaNewHead* metadata = reinterpret_cast<AkaNewHead*>(data) - 1;
	metadata->magicNumber = 0b10101010;
	metadata->type = type;
	metadata->category = category;
	//std::cout << "creating " << typeid(T).name() << " of type " << (int)EnumToValue(type) << " & " << (int)EnumToValue(category) << std::endl;
	return new (data) T(args...);
#else
	return new T(args...);
#endif
}

template <typename T>
void akaSafeDelete(const T* const & pointer)
{
	akaDelete(pointer);
	const_cast<T*&>(pointer) = nullptr;
}
template <typename T>
void akaDelete(const T* pointer)
{
#if defined(AKA_TRACK_MEMORY_ALLOCATIONS)
	if (pointer == nullptr)
		return;
	// Retrieve metadata from header.
	const AkaNewHead* metadata = reinterpret_cast<const AkaNewHead*>(pointer) - 1;
	AllocatorMemoryType type   = metadata->type;
	AllocatorCategory category = metadata->category;
	AKA_ASSERT(metadata->magicNumber == 0b10101010, "Invalid allocation passed to delete");
	//std::cout << "deleting " << typeid(T).name() << " of type " << (int)EnumToValue(type) << " & " << (int)EnumToValue(category) << std::endl;
	// Destroy data.
	pointer->~T();
	aka::mem::getAllocator(type, category).deallocate<AkaNewHead>(const_cast<T*>(pointer));
#else
	delete pointer;
#endif
}

template <typename T>
T* akaNewArray(size_t count, AllocatorMemoryType type, AllocatorCategory category)
{
#if defined(AKA_TRACK_MEMORY_ALLOCATIONS)
	T* data = aka::mem::getAllocator(type, category).allocate<T, AkaNewArrayHead>(count, aka::AllocatorFlags::None);
	// Store metadata in a header.
	AKA_ASSERT(countBitRange((uint32_t)count) < 32 - EnumBitCount<AllocatorMemoryType>() + EnumBitCount<AllocatorCategory>(), "Too many instances");
	AKA_ASSERT(EnumIsInRange(type), "Type invalid");
	AKA_ASSERT(EnumIsInRange(category), "Category invalid");
	AkaNewArrayHead* metadata = reinterpret_cast<AkaNewArrayHead*>(data) - 1;
	metadata->magicNumber = 0b10101010;
	metadata->type = type;
	metadata->category = category;
	metadata->count = count;
	//std::cout << "creating " << count << " " << typeid(T).name() << " bytes of type " << (int)EnumToValue(type) << " & " << (int)EnumToValue(category) << std::endl;
	// Construct data.
	std::uninitialized_default_construct(data, data + count);
	return data;
#else
	return new T[count]();
#endif
}

template <typename T>
void akaDeleteArray(const T*const & pointer)
{
#if defined(AKA_TRACK_MEMORY_ALLOCATIONS)
	if (pointer == nullptr)
		return;
	// Retrieve metadata
	const AkaNewArrayHead* metadata = reinterpret_cast<const AkaNewArrayHead*>(pointer) - 1;
	AllocatorMemoryType type = metadata->type;
	AllocatorCategory category = metadata->category;
	size_t count = metadata->count;
	AKA_ASSERT(metadata->magicNumber == 0b10101010, "Invalid allocation passed to delete");
	//std::cout << "deleting " << count << " " << typeid(T).name() << " bytes of type " << (int)EnumToValue(type) << " & " << (int)EnumToValue(category) << std::endl;
	// Deallocate data
	std::destroy(pointer, pointer + count);
	// const_cast here is OK cuz we are here to destroy it forever, bye !
	aka::mem::getAllocator(type, category).deallocate<AkaNewArrayHead>(const_cast<T*>(pointer));
	const_cast<T*&>(pointer) = nullptr; // Safe pointer invalidation
#else
	delete[] pointer;
#endif
}


}; // namespace mem
}; // namespace aka
