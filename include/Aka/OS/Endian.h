#pragma once

namespace aka {

struct Endianess {
	enum class Order {
		Little,
		Big
	};
	// Swap the endianess of the value passed.
	template<typename T>
	static T &swap(T& value);
	// Check if system is big endian
	static bool isBigEndian();
	// Check if system is little endian
	static bool isLittleEndian();
	// Check if given endian byte order is the same as system
	static bool same(Order order);
};

template<typename T> 
T &Endianess::swap(T& value)
{
	uint8_t* d = reinterpret_cast<uint8_t*>(&value);
	for (size_t i = 0; i < sizeof(T) / 2; i++)
	{
		uint8_t tmp = d[i];
		d[i] = d[sizeof(T) - i - 1];
		d[sizeof(T) - i - 1] = tmp;
	}
	return value;
}

};