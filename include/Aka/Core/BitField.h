#pragma once

#include <stdint.h>

#include <Aka/Core/Container/Vector.h>

namespace aka {

// Count bits in bitmask
uint32_t countbits32(uint32_t x);

// Find the most significant bit
uint32_t firstbithigh(uint32_t x);
// Find the least significant bit
uint32_t firstbitlow(uint32_t x);

// Generate a bitmask from a bit count
uint32_t bitmask(uint32_t bitcount);

struct BitField
{
	BitField();
	BitField(uint32_t bitcount);

	bool operator[](size_t index) const;

	void set(size_t index, bool value);
	bool check(size_t index) const;

	void clear();
private:
	Vector<uint8_t> m_container;
};

};