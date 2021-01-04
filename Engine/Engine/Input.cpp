#include "Input.h"

// TODO remove iostream only here for debug
#include <iostream>

namespace app {
namespace input {

Keyboard keyboard;
Cursor cursor;

void initialize()
{
	for (uint32_t iKey = 0; iKey < g_keyboardKeyCount; iKey++)
	{
		keyboard.pressed[iKey] = false;
		keyboard.timestamp[iKey] = 0;
	}
	for (uint32_t iKey = 0; iKey < g_mouseButtonCount; iKey++)
	{
		cursor.pressed[iKey] = false;
		cursor.timestamp[iKey] = 0;
	}
}

void on_key_down(Key key)
{
	keyboard.pressed[static_cast<int>(key)] = true;
	keyboard.timestamp[static_cast<int>(key)] = 0;
}

void on_key_up(Key key)
{
	keyboard.pressed[static_cast<int>(key)] = false;
	keyboard.timestamp[static_cast<int>(key)] = 0;
}

void on_mouse_button_down(Button button)
{
	std::cout << "down" << std::endl;
	cursor.pressed[static_cast<int>(button)] = true;
	cursor.timestamp[static_cast<int>(button)] = 0;
}

void on_mouse_button_up(Button button)
{
	std::cout << "up" << std::endl;
	cursor.pressed[static_cast<int>(button)] = false;
	cursor.timestamp[static_cast<int>(button)] = 0;
}

void on_mouse_move(float x, float y)
{
	cursor.position.x = x;
	cursor.position.y = y;
}

void on_mouse_scroll(float x, float y)
{
	cursor.scroll.x = x;
	cursor.scroll.y = y;
}

bool pressed(Key key)
{
	return keyboard.pressed[static_cast<int>(key)];
}

bool pressed(Button button)
{
	return cursor.pressed[static_cast<int>(button)];
}

Keyboard& getKeyboard()
{
	return keyboard;
}

Cursor& getCursor()
{
	return cursor;
}



}
}