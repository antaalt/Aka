#pragma once

#include <stdexcept>
#include <string>
#include <type_traits>

#include <Aka/Core/Config.h>

namespace aka {
namespace hash {

void fnv(size_t& hash, const void* data, size_t size);
size_t fnv(const void* data, size_t size);

// From boost.
template <typename T> void combine(std::size_t& s, const T& v)
{
	fnv(s, &v, sizeof(T));
}

}; // namespace hash
}; // namespace aka