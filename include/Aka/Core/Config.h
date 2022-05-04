#pragma once

#include <stdexcept>
#include <string>

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

template <typename T>
inline constexpr std::underlying_type_t<T> EnumToIntegral(T value)
{
	return static_cast<std::underlying_type_t<T>>(value);
}

};