#pragma once

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define GL_CHECK_RESULT(result)                \
{                                              \
	GLenum res = (result);                     \
	if (GL_NO_ERROR != res) {                  \
		char buffer[256];                      \
		snprintf(                              \
			buffer,                            \
			256,                               \
			"%s (%s at %s:%d)",                \
			glewGetErrorString(res),           \
			STRINGIFY(result),                 \
			__FILE__,                          \
			__LINE__                           \
		);                                     \
		std::cerr << buffer << std::endl;      \
		throw std::runtime_error(res, buffer); \
	}                                          \
}

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