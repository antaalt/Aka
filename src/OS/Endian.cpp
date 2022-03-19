#include <Aka/OS/Endian.h>
#include <Aka/Platform/Platform.h>

#if defined(AKA_PLATFORM_WINDOWS)
#include <stdlib.h>
#endif

namespace aka {

void Endian::swap16(uint16_t& value)
{
#if defined(AKA_PLATFORM_WINDOWS)
	value = _byteswap_ushort(value);
#else
	value = ((value >> 8) & 0xff) + ((value << 8) & 0xff00);
#endif
}

void Endian::swap32(uint32_t& value)
{
#if defined(AKA_PLATFORM_WINDOWS)
	value = _byteswap_ulong(value);
#else
	value = (value >> 24) + ((value >> 8) & 0xff00) + ((value << 8) & 0xff0000) + (value << 24);
#endif
}

void Endian::swap64(uint64_t& value)
{
#if defined(AKA_PLATFORM_WINDOWS)
	value = _byteswap_uint64(value);
#else
	value = ((value << 8) & 0xff00ff00ff00ff00ULL) | ((value >> 8) & 0x00ff00ff00ff00ffULL);
	value = ((value << 16) & 0xffff0000ffff0000ULL) | ((value >> 16) & 0x0000ffff0000ffffULL);
	value = (value << 32) | (value >> 32);
#endif
}

constexpr bool Endian::isBigEndian()
{
	constexpr uint32_t ui = 0x01020304;
	constexpr uint8_t ub = (const uint8_t&)ui;
	return ub == 0x01;
}

constexpr bool Endian::isMiddleEndian()
{
	constexpr uint32_t ui = 0x01020304;
	constexpr uint8_t ub = (const uint8_t&)ui;
	return ub == 0x02;
}

constexpr bool Endian::isLittleEndian()
{
	constexpr uint32_t ui = 0x01020304;
	constexpr uint8_t ub = (const uint8_t&)ui;
	return ub == 0x04;
}

bool Endian::same(Endianess order)
{
	return Endian::native() == order;
}

constexpr Endianess Endian::native()
{
	if constexpr (Endian::isBigEndian())
		return Endianess::Big;
	else if constexpr (Endian::isLittleEndian())
		return Endianess::Little;
	else if constexpr (Endian::isMiddleEndian())
		return Endianess::Middle;
	else
		return Endianess::Unknown;
}

};