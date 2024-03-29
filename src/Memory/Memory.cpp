#include <Aka/Memory/Memory.h>

namespace aka {

Memory::Memory(size_t size) :
	m_size(size), 
	m_data(Memory::alloc(size)) 
{
}

Memory::Memory(const Memory& memory) :
	Memory(memory.size())
{
	Memory::copy(m_data, memory.raw(), memory.size());
}

Memory& Memory::operator=(const Memory& memory)
{
	if (m_size != memory.size())
	{
		Memory::free(m_data);
		m_data = Memory::alloc(memory.size());
		m_size = memory.size();
	}
	Memory::copy(m_data, memory.raw(), memory.size());
	return *this;
}

Memory::Memory(Memory&& memory) :
	m_size(0),
	m_data(nullptr)
{
	std::swap(m_size, memory.m_size);
	std::swap(m_data, memory.m_data);
}

Memory& Memory::operator=(Memory&& memory)
{
	std::swap(m_size, memory.m_size);
	std::swap(m_data, memory.m_data);
	return *this;
}

Memory::~Memory()
{ 
	Memory::free(m_data);
}

size_t Memory::size() const
{ 
	return m_size; 
}

void* Memory::raw() const
{ 
	return m_data; 
}

void* Memory::alloc(size_t size)
{
	// TODO request os memory with brk / mmap
	return ::malloc(size);
}

void Memory::free(void* data)
{
	::free(data); 
}

void* Memory::copy(void* dst, const void* src, size_t size)
{
	return ::memcpy(dst, src, size);
}

void Memory::set(void* memory, uint8_t data, size_t size)
{
	::memset(memory, data, size);
}

int32_t Memory::compare(const void* lhs, const void* rhs, size_t size)
{
	return ::memcmp(lhs, rhs, size);
}

void Memory::zero(void* memory, size_t size)
{
	Memory::set(memory, 0, size);
}

bool Memory::operator==(const Memory& rhs)
{
	if (m_size != rhs.m_size)
		return false;
	return Memory::compare(m_data, rhs.m_data, m_size) == 0;
}

bool Memory::operator!=(const Memory& rhs)
{
	if (m_size == rhs.m_size)
		return false;
	return Memory::compare(m_data, rhs.m_data, m_size) != 0;
}



};