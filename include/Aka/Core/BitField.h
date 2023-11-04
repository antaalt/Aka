#pragma once

#include <stdint.h>
#include <iostream>

#include <Aka/Core/Enum.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/Core/Container/String.h>

namespace aka {

static const uint32_t BitNotFoundIndex = (uint32_t)-1;

// TODO should use this as input for following function ?
enum class bitset32 : uint32_t {};
enum class bitset64 : uint64_t {};

//AKA_IMPLEMENT_BITMASK_OPERATOR(bitset32);
//AKA_IMPLEMENT_BITMASK_OPERATOR(bitset64);

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
// Get the number of bits required to display this value.
uint32_t bitnum(uint32_t value);
// Check if given number is a power of two
bool isPowerOfTwo(uint32_t value);
bool isPowerOfTwo(uint64_t value);
// Find upper power of two for a given number
uint32_t findNextPowerOfTwo(uint32_t value);
uint64_t findNextPowerOfTwo(uint64_t value);
// Find lower power of two for a given number
uint32_t findPreviousPowerOfTwo(uint32_t value);
uint64_t findPreviousPowerOfTwo(uint64_t value);

// Convert number to a string of the underlying binary
std::ostream& operator<<(std::ostream&, const bitset32&);
std::ostream& operator<<(std::ostream&, const bitset64&);


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


constexpr uint32_t bitmask(uint32_t bitCount)
{
	return (1 << bitCount) - 1;
}

};