#pragma once

#include <Aka/Core/Debug.h>

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

	// Get the size of the memory block
	size_t size() const;
	// Get the raw pointer to it
	void* raw() const;

	// Allocate memory for given size
	static void* alloc(size_t size);
	// Deallocate memory
	static void free(void* data);
	// Copy memory
	static void* copy(void* dst, const void* src, size_t size);
private:
	size_t m_size;
	void* m_data;
};

};
