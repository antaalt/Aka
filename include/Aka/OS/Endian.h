#pragma once

#include <stdint.h>
#include <cstddef>

namespace aka {

enum class Endianess : uint8_t
{
	Unknown,

	Little,
	Big,
	Middle,

	Default = Little,
};

struct Endian 
{
	// Swap the endianess of the value passed.
	template<typename T> static void swap(T& value);
	// Swap 16 bits elements
	static void swap16(uint16_t& value);
	// Swap 32 bits elements
	static void swap32(uint32_t& value);
	// Swap 64 bits elements
	static void swap64(uint64_t& value);

	// Check if system is big endian
	static constexpr bool isBigEndian();
	// Check if system is middle endian
	static constexpr bool isMiddleEndian();
	// Check if system is little endian
	static constexpr bool isLittleEndian();

	// Check if given endian byte order is the same as system
	static bool same(Endianess order);
	// Get the native endianess
	static constexpr Endianess native();
};

template <typename T> inline void Endian::swap(T& value) {} // Do not swap if not needed.
template <> inline void Endian::swap(int16_t& value) { Endian::swap16(reinterpret_cast<uint16_t&>(value)); }
template <> inline void Endian::swap(int32_t& value) { Endian::swap32(reinterpret_cast<uint32_t&>(value)); }
template <> inline void Endian::swap(int64_t& value) { Endian::swap64(reinterpret_cast<uint64_t&>(value)); }
template <> inline void Endian::swap(uint16_t& value) { Endian::swap16(value); }
template <> inline void Endian::swap(uint32_t& value) { Endian::swap32(value); }
template <> inline void Endian::swap(uint64_t& value) { Endian::swap64(value); }
template <> inline void Endian::swap(float& value) { Endian::swap32(reinterpret_cast<uint32_t&>(value)); }
template <> inline void Endian::swap(double& value) { Endian::swap64(reinterpret_cast<uint64_t&>(value)); }
template <> inline void Endian::swap(char16_t& value) { Endian::swap16(reinterpret_cast<uint16_t&>(value)); }

};