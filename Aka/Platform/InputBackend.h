#pragma once

#include "Input.h"

namespace aka {

class InputBackend
{
public:
	static void initialize();
	static void destroy();
	static void update();
private:
	friend class PlatformBackend;
	static void onKeyDown(input::Key key);
	static void onKeyUp(input::Key key);
	static void onMouseButtonDown(input::Button button);
	static void onMouseButtonUp(input::Button button);
	static void onMouseMove(float x, float y);
	static void onMouseScroll(float x, float y);
	static input::Key getKeyFromScancode(int scancode);
	static input::Key getKeyFromPlatformKey(int key);
	static input::KeyboardLayout getKeyboardLayout();
};

}