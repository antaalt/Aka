#pragma once

#include <Aka/Core/Event.h>
#include <Aka/Core/Geometry.h>
#include <Aka/Platform/Input.h>

#include <set>

namespace aka {

struct Controller
{
	virtual ~Controller() {}
	virtual void update() = 0;
};

struct ButtonController : Controller
{
	void add(KeyboardKey key);
	void add(MouseButton button);
	void add(GamepadButton button);
	void add(GamepadAxis axis, float threshold);

	bool down() const;
	bool pressed() const;
	bool up() const;

	void update() override;

private:
	size_t m_down;
	size_t m_up;
	size_t m_pressed;

private:
	HashSet<KeyboardKey> m_keyboardKeys;
	HashSet<MouseButton> m_mouseButtons;
	HashSet<GamepadButton> m_gamepadButtons;
};

struct MotionController : Controller
{
	void addMouse();
	void add(GamepadAxis axis);
	void add(KeyboardKey left, KeyboardKey right, KeyboardKey up, KeyboardKey down);

	const Position& delta() const;

	void update() override;

private:
	Position m_delta;

private:
	struct KeyboardAxis {
		KeyboardKey left;
		KeyboardKey right;
		KeyboardKey up;
		KeyboardKey down;
		bool operator<(const KeyboardAxis& rhs) const;
	};

	bool m_mouseAxis;
	TreeSet<KeyboardAxis> m_keyboardAxis;
	TreeSet<GamepadAxis> m_gamepadAxis;
};

};