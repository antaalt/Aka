#pragma once

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#if defined(_DEBUG) || defined(DEBUG)

#if defined(_MSC_VER)
#include <cstdio>
#define DEBUG_BREAK __debugbreak()
#define PRINT(...) fprintf(stderr, __VA_ARGS__);
#elif defined(__CUDACC__)	
#define DEBUG_BREAK rtThrow(RT_EXCEPTION_USER)	
#define PRINT(...) rtPrintf(stderr, __VA_ARGS__);
#else
#include <cassert>
#include <cstdio>
#define DEBUG_BREAK assert(false)
#define PRINT(...) fprintf(stderr, __VA_ARGS__);
#endif

#define ASSERT(condition, message)						\
if (!(condition))									\
{													\
PRINT(\
	"Error : %s\nAssertion (%s) failed at %s:%d\n", \
	message, \
	(#condition), \
	__FILE__, \
	__LINE__);										\
	DEBUG_BREAK;									\
}

#else
#define ASSERT(condition, message) ((void)0)
#endif