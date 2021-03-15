#pragma once

#include <stdexcept>
#include <string>

#define AKA_STRINGIFY(x) #x
#define AKA_TOSTRING(x) AKA_STRINGIFY(x)

#if defined(_DEBUG) && !defined(DEBUG)
#define AKA_DEBUG
#define DEBUG
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

class RuntimeError : public std::exception
{
public:
	RuntimeError(const char* str);
	RuntimeError(const std::string& str);

	const char* what() const noexcept override;
private:
	std::string m_error;
};


class GraphicError : public std::exception
{
public:
	GraphicError(const char* str);
	GraphicError(const std::string& str);

	const char* what() const noexcept override;
private:
	std::string m_error;
};

};