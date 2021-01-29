#pragma once

#ifdef _WIN32 // Windows x64/x86
	#define AKA_WINDOWS
	#ifdef _WIN64 // Windows x64
		#define AKA_WINDOWS64
	#else // Windows x86
		#define AKA_WINDOWS32
		#error "Unsupported yet..."
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#define AKA_APPLE
	#error "Unsupported yet..."
#elif defined(__ANDROID__)
	#define AKA_ANDROID
	#error "Unsupported yet..."
#elif defined(__linux__)
	#define AKA_LINUX
	#error "Unsupported yet..."
#else
	#error "Unknown"
#endif

#if defined(AKA_USE_OPENGL)
	#define GLEW_NO_GLU
	#include <gl/glew.h>
	#include <gl/gl.h>
	#if defined(AKA_WINDOWS)
		#define NOMINMAX
		#define WIN32_LEAN_AND_MEAN
		#include <Windows.h>
	#endif
	#include <GLFW/glfw3.h>
#elif defined(AKA_USE_D3D11)
	#define GLFW_INCLUDE_NONE
	#define GLFW_EXPOSE_NATIVE_WIN32
	#if defined(AKA_WINDOWS)
		#define NOMINMAX
		#define WIN32_LEAN_AND_MEAN
		#include <Windows.h>
	#endif
	#include <GLFW/glfw3.h>
	#include <GLFW/glfw3native.h>
#endif

