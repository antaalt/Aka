#include <Aka/Platform/PlatformWindow.h>

namespace aka {

PlatformWindow::PlatformWindow(const PlatformWindowConfig& config) :
	m_keyboard{},
	m_mouse{},
	m_gamepads{},
	m_x(config.x),
	m_y(config.y),
	m_width(config.width),
	m_height(config.height),
	m_flags(config.flags),
	m_name(config.name),
	m_icon(config.icon),
	m_surface(gfx::SurfaceHandle::null),
	m_swapchain(gfx::SwapchainHandle::null)
{
}

PlatformWindow::~PlatformWindow()
{
}

void PlatformWindow::initialize(gfx::GraphicDevice* _device)
{
	initialize();
	m_surface = _device->createSurface(String::format("%sSurface", m_name.cstr()).cstr(), this);
	m_swapchain = _device->createSwapchain(String::format("%sSwapchain", m_name.cstr()).cstr(), m_surface, m_width, m_height, gfx::TextureFormat::BGRA8, gfx::SwapchainMode::Windowed, gfx::SwapchainType::Performance);
}
void PlatformWindow::shutdown(gfx::GraphicDevice* _device)
{
	shutdown();
	_device->destroy(m_swapchain);
	_device->destroy(m_surface);
}
const Mouse& PlatformWindow::mouse() const
{
	return m_mouse;
}
const Keyboard& PlatformWindow::keyboard() const
{
	return m_keyboard;
}
const Gamepad& PlatformWindow::gamepad(GamepadID gid) const
{
	auto it = m_gamepads.find(gid);
	if (it != m_gamepads.end())
		return it->second;
	throw std::runtime_error("Invalid gid.");
}
bool PlatformWindow::connected(GamepadID gid) const
{
	return m_gamepads.find(gid) != m_gamepads.end();
}
void PlatformWindow::onInputsUpdate()
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
void PlatformWindow::onKeyboardKeyDown(KeyboardKey key)
{
	m_keyboard.m_pressedCount++;
	m_keyboard.m_down[static_cast<int>(key)] = true;
	m_keyboard.m_pressed[static_cast<int>(key)] = true;
}
void PlatformWindow::onKeyboardKeyUp(KeyboardKey key)
{
	m_keyboard.m_pressedCount--;
	m_keyboard.m_up[static_cast<int>(key)] = true;
	m_keyboard.m_pressed[static_cast<int>(key)] = false;
}
void PlatformWindow::onMouseButtonDown(MouseButton button)
{
	m_mouse.m_down[static_cast<int>(button)] = true;
	m_mouse.m_pressed[static_cast<int>(button)] = true;
}
void PlatformWindow::onMouseButtonUp(MouseButton button)
{
	m_mouse.m_up[static_cast<int>(button)] = true;
	m_mouse.m_pressed[static_cast<int>(button)] = false;
}
void PlatformWindow::onMouseMotion(float x, float y)
{
	m_mouse.m_delta.x = x - m_mouse.m_position.x;
	m_mouse.m_delta.y = y - m_mouse.m_position.y;
	m_mouse.m_position.x = x;
	m_mouse.m_position.y = y;
}
void PlatformWindow::onMouseScroll(float x, float y)
{
	m_mouse.m_scroll.x = x;
	m_mouse.m_scroll.y = y;
}
void PlatformWindow::onMouseEnter()
{
	m_mouse.m_focus = true;
}
void PlatformWindow::onMouseLeave()
{
	m_mouse.m_focus = false;
}
void PlatformWindow::onGamepadConnected(GamepadID gid, const char* name)
{
	auto it = m_gamepads.insert(std::make_pair(gid, Gamepad{}));
	if (it.second)
		it.first->second.m_name = name;
}
void PlatformWindow::onGamepadDisconnected(GamepadID gid)
{
	auto it = m_gamepads.find(gid);
	if (it != m_gamepads.end())
		m_gamepads.erase(it);
}
void PlatformWindow::onGamepadButtonDown(GamepadID gid, GamepadButton button)
{
	AKA_ASSERT(connected(gid), "No gamepad");
	m_gamepads[gid].m_down[static_cast<int>(button)] = true;
	m_gamepads[gid].m_pressed[static_cast<int>(button)] = true;
}
void PlatformWindow::onGamepadButtonUp(GamepadID gid, GamepadButton button)
{
	AKA_ASSERT(connected(gid), "No gamepad");
	m_gamepads[gid].m_up[static_cast<int>(button)] = true;
	m_gamepads[gid].m_pressed[static_cast<int>(button)] = false;
}
void PlatformWindow::onGamepadAxisMotion(GamepadID gid, GamepadAxis axis, const Position& value)
{
	AKA_ASSERT(connected(gid), "No gamepad");
	m_gamepads[gid].m_axes[(int)axis] = value;
}
uint32_t PlatformWindow::width() const
{
	return m_width;
}
uint32_t PlatformWindow::height() const
{
	return m_height;
}
int32_t PlatformWindow::x() const
{
	return m_x;
}
int32_t PlatformWindow::y() const
{
	return m_y;
}
PlatformWindowFlag PlatformWindow::flags() const
{
	return m_flags;
}
gfx::SurfaceHandle PlatformWindow::surface() const
{
	return m_surface;
}
gfx::SwapchainHandle PlatformWindow::swapchain() const
{
	return m_swapchain;
}
}