#include <Aka/Core/Controller.h>

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
	m_down = 0;
	m_up = 0;
	for (KeyboardKey key : m_keyboardKeys)
	{
		if (Keyboard::down(key))
		{
			m_down++;
			m_pressed++;
		}
		if (Keyboard::up(key))
		{
			m_up++;
			m_pressed--;
		}
	}
	for (MouseButton button : m_mouseButtons)
	{
		if (Mouse::down(button))
		{
			m_down++;
			m_pressed++;
		}
		if (Mouse::up(button))
		{
			m_up++;
			m_pressed--;
		}
	}
	GamepadID gid = Gamepad::get();
	for (GamepadButton button : m_gamepadButtons)
	{
		if (Gamepad::down(gid, button))
		{
			m_down++;
			m_pressed++;
		}
		if (Gamepad::up(gid, button))
		{
			m_up++;
			m_pressed--;
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
	m_delta = { 0.f };
	if (m_mouseAxis)
	{
		const Position& pos = Mouse::delta();
		m_delta.x = abs(pos.x) > abs(m_delta.x) ? pos.x : m_delta.x;
		m_delta.y = abs(pos.y) > abs(m_delta.y) ? pos.y : m_delta.y;
	}
	GamepadID gid = Gamepad::get();
	for (GamepadAxis axis : m_gamepadAxis)
	{
		const Position& pos = Gamepad::axis(gid, axis);
		if (abs(pos.x) > 0.3f)
			m_delta.x = abs(pos.x) > abs(m_delta.x) ? pos.x : m_delta.x;
		if (abs(pos.y) > 0.3f)
			m_delta.y = abs(pos.y) > abs(m_delta.y) ? pos.y : m_delta.y;
	}
	for (const KeyboardAxis& axis : m_keyboardAxis)
	{
		Position pos;
		pos.x = static_cast<float>(Keyboard::pressed(axis.right) - Keyboard::pressed(axis.left));
		pos.y = static_cast<float>(Keyboard::pressed(axis.up) - Keyboard::pressed(axis.down));
		m_delta.x = abs(pos.x) > abs(m_delta.x) ? pos.x : m_delta.x;
		m_delta.y = abs(pos.y) > abs(m_delta.y) ? pos.y : m_delta.y;
	}
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