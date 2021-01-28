#pragma once

#include "../../Core/ECS/Component.h"
#include "../../Platform/Input/Input.h"

namespace aka {

struct Speed
{
	Speed(float metric) : m_speed(metric) {}

	// Generate speed in m/s
	static Speed metric(float metric) { return Speed(metric); }
	// Generate speed in pixel/s (16 pixels -> 1 m)
	static Speed pixel(float pixel) { return Speed(pixel / 16.f); }

	float metric() const { return m_speed; }
	float pixel() const { return m_speed * 16.f; }

private:
	float m_speed;
};

struct Control
{
	Control() {}
	Control(input::Key key) { m_keys.push_back(key); }

	void update(Time::Unit deltaTime) {
		m_down = false;
		m_up = false;
		m_pressed = false;
		for (input::Key& key : m_keys)
		{
			m_down |= input::down(key);
			m_up |= input::up(key);
			m_pressed |= input::pressed(key);
		}
		for (input::Button& button : m_buttons)
		{
			m_down |= input::down(button);
			m_up |= input::up(button);
			m_pressed |= input::pressed(button);
		}
	}
	// Is the key pressed
	bool pressed() const { return m_pressed; }
	// Is the key down
	bool down() const { return m_down; }
	// Is the key down
	bool up() const { return m_up; }
private:
	bool m_down;
	bool m_up;
	bool m_pressed;
	std::vector<input::Key> m_keys;
	std::vector<input::Button> m_buttons;
};

struct Player : public Component
{
	int coin = 0;

	enum class State {
		Idle,
		Walking,
		Jumping,
		DoubleJumping,
		Falling
	};
	State state;
	Speed speed = Speed(6.f);
	Control jump;
	Control left;
	Control right;
	bool ground;
};

}