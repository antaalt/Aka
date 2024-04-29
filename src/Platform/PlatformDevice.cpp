#include <Aka/Platform/PlatformDevice.h>

#include "GLFW3/PlatformGLFW3.h"
#include <Aka/Core/Application.h>

namespace aka {

PlatformFlag aka::operator|(const PlatformFlag& lhs, const PlatformFlag& rhs)
{
	return static_cast<PlatformFlag>((int)lhs | (int)rhs);
}

PlatformFlag aka::operator&(const PlatformFlag& lhs, const PlatformFlag& rhs)
{
	return static_cast<PlatformFlag>((int)lhs & (int)rhs);
}

PlatformFlag operator~(const PlatformFlag& flag)
{
	return static_cast<PlatformFlag>(~(int)flag);
}

PlatformDevice* PlatformDevice::create(const PlatformConfig& config)
{
#if defined(AKA_USE_GLFW3)
	return mem::akaNew<PlatformGLFW3>(AllocatorMemoryType::Persistent, AllocatorCategory::Default, config);
#else
	Logger::critical("No platform defined.");
	return nullptr;
#endif
}

void PlatformDevice::destroy(PlatformDevice* device)
{
	mem::akaDelete(device);
}

PlatformDevice::PlatformDevice(const PlatformConfig& config) :
	m_width(config.width),
	m_height(config.height),
	m_x(config.x),
	m_y(config.y),
	m_flags(config.flags),
	m_keyboard{},
	m_mouse{},
	m_gamepads{}
{
}

PlatformDevice::~PlatformDevice()
{
}

uint32_t PlatformDevice::width() const
{
	return m_width;
}
uint32_t PlatformDevice::height() const
{
	return m_height;
}
int32_t PlatformDevice::x() const
{
	return m_x;
}
int32_t PlatformDevice::y() const
{
	return m_y;
}
PlatformFlag aka::PlatformDevice::flags() const
{
	return m_flags;
}
const Mouse& PlatformDevice::mouse() const
{
	return m_mouse;
}
const Keyboard& PlatformDevice::keyboard() const
{
	return m_keyboard;
}
const Gamepad& PlatformDevice::gamepad(GamepadID gid) const
{
	auto it = m_gamepads.find(gid);
	if (it != m_gamepads.end())
		return it->second;
	throw std::runtime_error("Invalid gid.");
}
bool PlatformDevice::connected(GamepadID gid) const
{
	return m_gamepads.find(gid) != m_gamepads.end();
}
void PlatformDevice::onInputsUpdate()
{
	// Reset keyboards keys up & down
	for (uint32_t iKey = 0; iKey < getKeyboardKeyCount(); iKey++)
		m_keyboard.m_down[iKey] = false;
	for (uint32_t iKey = 0; iKey < getKeyboardKeyCount(); iKey++)
		m_keyboard.m_up[iKey] = false;

	// Reset mouse buttons up & down
	for (uint32_t iButton = 0; iButton < getMouseButtonCount(); iButton++)
		m_mouse.m_down[iButton] = false;
	for (uint32_t iButton = 0; iButton < getMouseButtonCount(); iButton++)
		m_mouse.m_up[iButton] = false;
	m_mouse.m_delta = { 0.f };
	m_mouse.m_scroll = { 0.f };

	// Reset gamepad buttons up & down
	for (auto& g : m_gamepads)
	{
		for (uint32_t iButton = 0; iButton < getGamepadButtonCount(); iButton++)
			g.second.m_down[iButton] = false;
		for (uint32_t iButton = 0; iButton < getGamepadButtonCount(); iButton++)
			g.second.m_up[iButton] = false;
	}
}
void PlatformDevice::onKeyboardKeyDown(KeyboardKey key)
{
	m_keyboard.m_pressedCount++;
	m_keyboard.m_down[static_cast<int>(key)] = true;
	m_keyboard.m_pressed[static_cast<int>(key)] = true;
}
void PlatformDevice::onKeyboardKeyUp(KeyboardKey key)
{
	m_keyboard.m_pressedCount--;
	m_keyboard.m_up[static_cast<int>(key)] = true;
	m_keyboard.m_pressed[static_cast<int>(key)] = false;
}
void PlatformDevice::onMouseButtonDown(MouseButton button)
{
	m_mouse.m_down[static_cast<int>(button)] = true;
	m_mouse.m_pressed[static_cast<int>(button)] = true;
}
void PlatformDevice::onMouseButtonUp(MouseButton button)
{
	m_mouse.m_up[static_cast<int>(button)] = true;
	m_mouse.m_pressed[static_cast<int>(button)] = false;
}
void PlatformDevice::onMouseMotion(float x, float y)
{
	m_mouse.m_delta.x = x - m_mouse.m_position.x;
	m_mouse.m_delta.y = y - m_mouse.m_position.y;
	m_mouse.m_position.x = x;
	m_mouse.m_position.y = y;
}
void PlatformDevice::onMouseScroll(float x, float y)
{
	m_mouse.m_scroll.x = x;
	m_mouse.m_scroll.y = y;
}
void PlatformDevice::onMouseEnter()
{
	m_mouse.m_focus = true;
}
void PlatformDevice::onMouseLeave()
{
	m_mouse.m_focus = false;
}
void PlatformDevice::onGamepadConnected(GamepadID gid, const char* name)
{
	auto it = m_gamepads.insert(std::make_pair(gid, Gamepad {}));
	if (it.second)
		it.first->second.m_name = name;
}
void PlatformDevice::onGamepadDisconnected(GamepadID gid)
{
	auto it = m_gamepads.find(gid);
	if (it != m_gamepads.end())
		m_gamepads.erase(it);
}
void PlatformDevice::onGamepadButtonDown(GamepadID gid, GamepadButton button)
{
	AKA_ASSERT(connected(gid), "No gamepad");
	m_gamepads[gid].m_down[static_cast<int>(button)] = true;
	m_gamepads[gid].m_pressed[static_cast<int>(button)] = true;
}
void PlatformDevice::onGamepadButtonUp(GamepadID gid, GamepadButton button)
{
	AKA_ASSERT(connected(gid), "No gamepad");
	m_gamepads[gid].m_up[static_cast<int>(button)] = true;
	m_gamepads[gid].m_pressed[static_cast<int>(button)] = false;
}
void PlatformDevice::onGamepadAxisMotion(GamepadID gid, GamepadAxis axis, const Position& value)
{
	AKA_ASSERT(connected(gid), "No gamepad");
	m_gamepads[gid].m_axes[(int)axis] = value;
}

};