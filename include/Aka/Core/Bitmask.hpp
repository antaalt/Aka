#pragma once

#include <stdint.h>
#include <iostream>

#include <Aka/Core/Enum.h>

namespace aka {

// TODO should use this as input for following function ?
enum class bitset32 : uint32_t {};
enum class bitset64 : uint64_t {};

//AKA_IMPLEMENT_BITMASK_OPERATOR(bitset32);
//AKA_IMPLEMENT_BITMASK_OPERATOR(bitset64);

// Count bits in bitmask
uint32_t countBitSet(uint32_t x);
uint32_t countBitSet(uint64_t x);

// Find the most significant bit. Return 32 or 64 if no bit set.
uint32_t countLeadingZero(uint32_t x);
uint32_t countLeadingZero(uint64_t x);
// Find the least significant bit. Return 32 or 64 if no bit set.
uint32_t countTrailingZero(uint32_t x);
uint32_t countTrailingZero(uint64_t x);
// Find the most significant bit
uint32_t findMostSignificantBit(uint32_t x);
uint32_t findMostSignificantBit(uint64_t x);
// Find the least significant bit
uint32_t findLeastSignificantBit(uint32_t x);
uint32_t findLeastSignificantBit(uint64_t x);

// Generate a bitmask from a bit count
template <typename T> constexpr T bitmask(uint32_t bitCount);
// Generate a bitmask from a type
template <typename T> constexpr T bitmask();
// Get number of bits for a given type
template <typename T> constexpr uint32_t countBitSize();
// Get the number of bits required to display this value.
uint32_t countBitRange(uint32_t value);
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

template <typename T> constexpr T bitmask(uint32_t bitCount)
{
	return static_cast<T>(-(bitCount != 0)) & (static_cast<T>(-1) >> ((sizeof(T) * CHAR_BIT) - bitCount));
}

template <typename T> constexpr T bitmask()
{
	return bitcount<T>(sizeof(T) * CHAR_BIT);
}

template <typename T> constexpr uint32_t countBitSize()
{
	return sizeof(T) * CHAR_BIT;
}

};