#include <Aka/Core/BitField.h>

#include <Aka/Core/Bitmask.hpp>

namespace aka {

BitField::BitField() :
	BitField(32)
{
}

BitField::BitField(uint32_t bitCount) :
	m_container((bitCount + 7) / 8, 0)
{
}

bool BitField::operator[](size_t index) const
{
	return (m_container[index / 8] >> (index & bitmask(8))) & 0x1;
}

void BitField::set(size_t index)
{
	m_container[index / 8] |= 1U << (index & bitmask(8));
}

void BitField::set(size_t index, bool value)
{
	m_container[index / 8] = (m_container[index / 8] & ~(static_cast<uint8_t>(1) << index)) | (static_cast<uint8_t>(value) << index);
}

void BitField::clear(size_t index)
{
	m_container[index / 8] &= ~(1U << (index & bitmask(8)));
}

void BitField::toggle(size_t index)
{
	m_container[index / 8] ^= 1U << (index & bitmask(8));
}

bool BitField::check(size_t index) const
{
	return (m_container[index / 8] >> (index & bitmask(8))) & 0x1;
}
void BitField::resize(size_t bitCount)
{
	m_container.resize((bitCount + 7) / 8);
}
void BitField::reserve(size_t bitCount)
{
	m_container.reserve((bitCount + 7) / 8);
}

void BitField::clear()
{
	for (uint8_t& byte : m_container)
		byte = 0;
}

};