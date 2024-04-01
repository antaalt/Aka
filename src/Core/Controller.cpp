#include <Aka/Core/Controller.h>

#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Core/Application.h>

namespace aka {

void ButtonController::add(KeyboardKey key)
{
	m_keyboardKeys.insert(key);
}

void ButtonController::add(MouseButton button)
{
	m_mouseButtons.insert(button);
}

void ButtonController::add(GamepadButton button)
{
	m_gamepadButtons.insert(button);
}

void ButtonController::add(GamepadAxis axis, float threshold)
{
	throw std::runtime_error("Not implemented");
}

bool ButtonController::down() const
{
	return m_down > 0;
}

bool ButtonController::pressed() const
{
	return m_pressed > 0;
}

bool ButtonController::up() const
{
	return m_up > 0;
}

void ButtonController::update()
{
	Application* app = Application::app();
	PlatformDevice* platform = app->platform();
	const Keyboard& keyboard = platform->keyboard();
	const Mouse& mouse = platform->mouse();
	m_down = 0;
	m_up = 0;
	for (KeyboardKey key : m_keyboardKeys)
	{
		if (keyboard.down(key))
		{
			m_down++;
			m_pressed++;
		}
		if (keyboard.up(key))
		{
			m_up++;
			m_pressed--;
		}
	}
	for (MouseButton button : m_mouseButtons)
	{
		if (mouse.down(button))
		{
			m_down++;
			m_pressed++;
		}
		if (mouse.up(button))
		{
			m_up++;
			m_pressed--;
		}
	}
	// TODO all gid ?
	GamepadID gid = (GamepadID)0;
	if (platform->connected(gid))
	{
		const Gamepad& gamepad = platform->gamepad(gid);
		for (GamepadButton button : m_gamepadButtons)
		{
			if (gamepad.down(gid, button))
			{
				m_down++;
				m_pressed++;
			}
			if (gamepad.up(gid, button))
			{
				m_up++;
				m_pressed--;
			}
		}
	}
}

void MotionController::addMouse()
{
	m_mouseAxis = true;
}

void MotionController::add(GamepadAxis axis)
{
	m_gamepadAxis.insert(axis);
}

void MotionController::add(KeyboardKey left, KeyboardKey right, KeyboardKey up, KeyboardKey down)
{
	m_keyboardAxis.insert(KeyboardAxis{ left, right, up, down });
}

const Position& MotionController::delta() const
{
	return m_delta;
}

void MotionController::update()
{
	Application* app = Application::app();
	PlatformDevice* platform = app->platform();
	const Keyboard& keyboard = platform->keyboard();
	const Mouse& mouse = platform->mouse();
	m_delta = { 0.f };
	if (m_mouseAxis)
	{
		const Position& pos = mouse.delta();
		m_delta.x += pos.x;
		m_delta.y += pos.y;
	}
	GamepadID gid = (GamepadID)0;
	if (platform->connected(gid))
	{
		const Gamepad& gamepad = platform->gamepad(gid);
		for (GamepadAxis axis : m_gamepadAxis)
		{
			const Position& pos = gamepad.axis(gid, axis);
			m_delta.x += pos.x;
			m_delta.y += pos.y;
		}
	}
	for (const KeyboardAxis& axis : m_keyboardAxis)
	{
		m_delta.x += static_cast<float>(keyboard.pressed(axis.right) - keyboard.pressed(axis.left));
		m_delta.y += static_cast<float>(keyboard.pressed(axis.up) - keyboard.pressed(axis.down));
	}
	// threshold
	if (abs(m_delta.x) < 0.3f)
		m_delta.x = 0.f;
	else
		m_delta.x = clamp(m_delta.x, -1.f, 1.f);
	if (abs(m_delta.y) < 0.3f)
		m_delta.y = 0.f;
	else
		m_delta.y = clamp(m_delta.y, -1.f, 1.f);
}

bool MotionController::KeyboardAxis::operator<(const MotionController::KeyboardAxis& rhs) const
{
	if (left < rhs.left) return true;
	if (right < rhs.right) return true;
	if (up < rhs.up) return true;
	if (down < rhs.down) return true;
	return false;
}

}