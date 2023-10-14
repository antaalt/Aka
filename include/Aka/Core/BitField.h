#pragma once

#include <stdint.h>

#include <Aka/Core/Container/Vector.h>

namespace aka {

static const uint32_t BitNotFoundIndex = (uint32_t)-1;

// Count bits in bitmask
uint32_t countBitSet(uint32_t x);
uint32_t countBitSet(uint64_t x);

// Find the most significant bit. Return BitNotFoundIndex if not bit set
uint32_t countLeadingZero(uint32_t x);
uint32_t countLeadingZero(uint64_t x);
// Find the least significant bit. Return BitNotFoundIndex if not bit set
uint32_t countTrailingZero(uint32_t x);
uint32_t countTrailingZero(uint64_t x);

// Generate a bitmask from a bit count
constexpr uint32_t bitmask(uint32_t bitCount);

struct BitField
{
	BitField();
	BitField(uint32_t bitCount);

	bool operator[](size_t index) const;

	void set(size_t index);
	void set(size_t index, bool value);
	void clear(size_t index);
	void toggle(size_t index);
	bool check(size_t index) const;

	void resize(size_t bitCount);
	void reserve(size_t bitCount);

	void clear();
private:
	Vector<uint8_t> m_container;
};

};