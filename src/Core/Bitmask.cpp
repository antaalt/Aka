#include <Aka/Core/Bitmask.hpp>

namespace aka {

uint32_t countBitSet(uint32_t x)
{
	// popcount equivalent
	// http://graphics.stanford.edu/%7Eseander/bithacks.html
#if defined(AKA_PLATFORM_WINDOWS)
	return __popcnt(x);
#elif defined(AKA_ENVIRONMENT64)
	// This method require compilation in 64 bit
	uint32_t	c = ((x & 0xfff) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1f;
	c += (((x & 0xfff000) >> 12) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1f;
	c += ((x >> 24) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1f;
	return c;
#else
	uint32_t c = 0;
	for (; x; c++)
		x &= x - 1; // clear the least significant bit set
	return c;
#endif
}

uint32_t countBitSet(uint64_t x)
{
#if defined(AKA_PLATFORM_WINDOWS)
	return (uint32_t)__popcnt64(x);
#else
	uint64_t c = 0;
	for (; x; c++)
		x &= x - 1; // clear the least significant bit set
	return c;
#endif
}
// Could check this for cross platform compatibility 
// https://github.com/mpdn/bitcount
uint32_t countLeadingZero(uint32_t x)
{
#if defined(AKA_PLATFORM_WINDOWS)
	unsigned long result = 0;
	if (_BitScanReverse(&result, x))
		return 31 - result;
	else
		return 32;
#else
	// Count leading zero
	return __builtin_clz(x); // GCC
#endif
}
uint32_t countTrailingZero(uint32_t x)
{
#if defined(AKA_PLATFORM_WINDOWS)
	unsigned long result = 0;
	if (_BitScanForward(&result, x))
		return result;
	else
		return 32;
#else
	// Count trailing zero
	return __builtin_ctz(x); // GCC
#endif
}
uint32_t countLeadingZero(uint64_t x)
{
#if defined(AKA_PLATFORM_WINDOWS)
	unsigned long result = 0;
	if (_BitScanReverse64(&result, x))
		return 63 - result;
	else
		return 64;
#else
	// Count leading zero
	return __builtin_clzll(x); // GCC
#endif
}
uint32_t countTrailingZero(uint64_t x)
{
#if defined(AKA_PLATFORM_WINDOWS)
	unsigned long result = 0;
	if (_BitScanForward64(&result, x))
		return result;
	else
		return 64;
#else
	// Count trailing zero
	return __builtin_ctzll(x); // GCC
#endif
}

uint32_t bitnum(uint32_t value)
{
	return 32 - countLeadingZero(value);
}
bool isPowerOfTwo(uint32_t value)
{
	return value && !(value & (value - 1U));
}
bool isPowerOfTwo(uint64_t value)
{
	return value && !(value & (value - 1ULL));
}
uint32_t findNextPowerOfTwo(uint32_t value)
{
	// Could simply call 31 - countLeadingZero(value) but watch out for return value with 0
	// https://graphics.stanford.edu/%7Eseander/bithacks.html#RoundUpPowerOf2
	value--;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value++;
	return value;
}
uint64_t findNextPowerOfTwo(uint64_t value)
{
	value--;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value |= value >> 32;
	value++;
	return value;
}
uint32_t findPreviousPowerOfTwo(uint32_t value)
{
	value |= (value >> 1);
	value |= (value >> 2);
	value |= (value >> 4);
	value |= (value >> 8);
	value |= (value >> 16);
	return value - (value >> 1);
}
uint64_t findPreviousPowerOfTwo(uint64_t value)
{
	value |= (value >> 1);
	value |= (value >> 2);
	value |= (value >> 4);
	value |= (value >> 8);
	value |= (value >> 16);
	value |= (value >> 32);
	return value - (value >> 1);
}

std::ostream& operator<<(std::ostream& os, const bitset32& value)
{
	for (uint32_t i = 32; i-- > 0;)
	{
		// TODO add shiftable operators.
		os << ((static_cast<uint32_t>(value) >> i) & 0x1) ? "1" : "0";
	}
	return os;
}
std::ostream& operator<<(std::ostream& os, const bitset64& value)
{
	for (uint32_t i = 64; i-- > 0;)
	{
		os << ((static_cast<uint64_t>(value) >> i) & 0x1) ? "1" : "0";
	}
	return os;
}

};