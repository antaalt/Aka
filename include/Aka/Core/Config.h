#pragma once

#include <stdexcept>
#include <string>
#include <type_traits>

#define AKA_NO_VTABLE __declspec(novtable)

#define AKA_STRINGIFY(x) #x
#define AKA_TOSTRING(x) AKA_STRINGIFY(x)

// Check for debug compiler define
#if defined(_DEBUG) || defined(DEBUG) || defined(DBG)
#define AKA_DEBUG
#else
#define AKA_RELEASE
#endif

#if defined(AKA_DEBUG)
	#if defined(_MSC_VER)
		#include <cstdio>
		#define AKA_DEBUG_BREAK __debugbreak()
		#define AKA_PRINT(...) fprintf(stderr, __VA_ARGS__);
	#elif defined(__CUDACC__)	
		#define AKA_DEBUG_BREAK rtThrow(RT_EXCEPTION_USER)	
		#define AKA_PRINT(...) rtPrintf(stderr, __VA_ARGS__);
	#else
		#include <cassert>
		#include <cstdio>
		#define AKA_DEBUG_BREAK assert(false)
		#define AKA_PRINT(...) fprintf(stderr, __VA_ARGS__);
	#endif

	#define AKA_ASSERT(condition, message)              \
	if (!(condition))									\
	{													\
		AKA_PRINT(                                      \
		"Error : %s\nAssertion (%s) failed at %s:%d\n", \
		message,                                        \
		(#condition),                                   \
		__FILE__,                                       \
		__LINE__);										\
		AKA_DEBUG_BREAK;								\
	}
#else
	#define AKA_ASSERT(condition, message) ((void)0)
#endif

namespace aka {

// Hash
template <typename T> struct Hash;
template <typename T> void hashCombine(std::size_t& s, const T& v);

// Type traits
template <typename T> using UnderlyingType = ::std::underlying_type_t<T>;
template <typename T> constexpr UnderlyingType<T> EnumToIntegral(T value);

// Containers
//template <typename T> using Vector = ::std::vector<T>;
//template <typename T> using TreeMap = ::std::map<T>;
//template <typename T> using HashMap = ::std::unordered_map<T>;

template <typename T>
void hashCombine(std::size_t& s, const T& v)
{
	std::hash<T> h;
	s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}

template <typename T>
struct Hash
{
	size_t operator()(const T& data)
	{
		return std::hash<T>(data);
	}
};

template <typename T>
inline constexpr UnderlyingType<T> EnumToIntegral(T value)
{
	return static_cast<UnderlyingType<T>>(value);
}

};