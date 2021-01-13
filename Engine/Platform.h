#pragma once

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

