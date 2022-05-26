#pragma once

// AKA_PLATFORM_XXX
// AKA_ENVIRONMENT_XXX
#ifdef _WIN32 // Windows x64/x86
	#define AKA_PLATFORM_WINDOWS
	#ifdef _WIN64 // Windows x64
		#define AKA_ENVIRONMENT64
	#else // Windows x86
		#define AKA_ENVIRONMENT32
		#error "Unsupported yet..."
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#define AKA_PLATFORM_APPLE
	#error "Unsupported yet..."
#elif defined(__ANDROID__)
	#define AKA_PLATFORM_ANDROID
	#error "Unsupported yet..."
#elif defined(__linux__)
	#define AKA_PLATFORM_LINUX
	#if __x86_64__ || __ppc64__
		#define AKA_ENVIRONMENT64
	#else
		#define AKA_ENVIRONMENT32
	#endif
#else
	#error "Unknown"
#endif