#pragma once

#include <stdexcept>
#include <string>
#include <type_traits>

#include <Aka/Platform/Platform.h>

#define AKA_NO_VTABLE __declspec(novtable)

#define AKA_STRINGIFY(x) #x
#define AKA_TOSTRING(x) AKA_STRINGIFY(x)

#define AKA_PPCAT_EXPANDED(A, B) A ## B
#define AKA_PPCAT(A, B) AKA_PPCAT_EXPANDED(A, B)

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

	#define AKA_ASSERT(condition, message)				\
	do													\
	{													\
		if (condition) break;							\
		AKA_PRINT(										\
		"Error : %s\nAssertion (%s) failed at %s:%d\n", \
		message,										\
		(#condition),									\
		__FILE__,										\
		__LINE__);										\
		AKA_DEBUG_BREAK;								\
	} while(0)
#else
	#define AKA_ASSERT(condition, message) ((void)0)
#endif


#define AKA_NOT_IMPLEMENTED AKA_ASSERT(false, "Feature not implemented")
#define AKA_UNREACHABLE AKA_ASSERT(false, "Code unreachable reached.")

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


void hash(size_t& hash, const void* data, size_t size);
size_t hash(const void* data, size_t size);

template <typename T>
void hash(size_t& s, const T& v)
{
	hash(s, &v, sizeof(T));
}

template <typename T>
size_t hash(const T& v)
{
	return hash(&v, sizeof(T));
}

template <typename T>
void hashCombine(std::size_t& s, const T& v)
{
	std::hash<T> h;
	s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}

};