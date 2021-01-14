#pragma once

#ifdef _WIN32 // Windows x64/x86
	#ifdef _WIN64 // Windows x64
	#else // Windows x86
		#error "Unsupported yet..."
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#error "Unsupported yet..."
#elif defined(__ANDROID__)
	#error "Unsupported yet..."
#elif defined(__linux__)
	#error "Unsupported yet..."
#else
	#error "Unknown"
#endif

#include "Graphic.h"
#include "Input.h"
#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif
#include <GLFW/glfw3.h>

namespace app {

void initKeyboard();
input::Key getKeyFromScancode(unsigned int scancode);
input::KeyboardLayout getKeyboardLayout();

}

