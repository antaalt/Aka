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

PlatformDevice::PlatformDevice(const PlatformConfig& config) :
	m_width(config.width),
	m_height(config.height),
	m_x(config.x),
	m_y(config.y),
	m_flags(config.flags)
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

static PlatformDevice* s_device = nullptr;

void PlatformBackend::initialize(const Config& config)
{
	PlatformConfig cfg;
	cfg.width = config.width;
	cfg.height = config.height;
	cfg.icon.bytes = (byte_t*)config.icon.data();
	cfg.icon.size = config.icon.width();
	cfg.name = config.name;
	s_device = new PlatformGLFW3(cfg);
}
void PlatformBackend::destroy()
{
	delete s_device;
	s_device = nullptr;
}
PlatformDevice* PlatformBackend::get()
{
	return s_device;
}

};