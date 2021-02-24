#include <Aka/OS/Endian.h>
#include <Aka/Platform/Platform.h>

#include <stdint.h>

namespace aka {

bool Endianess::isBigEndian()
{
	union {
		uint32_t i;
		char c[4];
	} bint = { 0x01020304 };
	return bint.c[0] == 1;
}

bool Endianess::isLittleEndian()
{
	union {
		uint32_t i;
		char c[4];
	} bint = { 0x01020304 };
	return bint.c[0] == 4;
}

bool Endianess::same(Endianess::Order order)
{
	return ((order == Order::Big) && isBigEndian()) || ((order == Order::Little) && isLittleEndian());
}

};