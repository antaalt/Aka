#include <Aka/Graphic/GraphicDevice.h>

namespace aka {

GraphicFlag operator|(const GraphicFlag& lhs, const GraphicFlag& rhs)
{
	return static_cast<GraphicFlag>((int)lhs | (int)rhs);
}
GraphicFlag operator&(const GraphicFlag& lhs, const GraphicFlag& rhs)
{
	return static_cast<GraphicFlag>((int)lhs & (int)rhs);
}
GraphicFlag operator~(const GraphicFlag& flag)
{
	return static_cast<GraphicFlag>(~(int)flag);
}

GraphicDevice::GraphicDevice(const GraphicConfig& config) :
	m_flags(config.flags),
	m_features{},
	m_backbuffer(nullptr)
{
}

GraphicDevice::~GraphicDevice()
{
}

const GraphicAPI& GraphicDevice::api() const
{
	return m_settings.api;
}

const GraphicDeviceFeatures& GraphicDevice::features() const
{
	return m_features;
}

GraphicFlag GraphicDevice::flags() const
{
	return m_flags;
}

const GraphicDeviceSettings& GraphicDevice::settings() const
{
	return m_settings;
}

const GraphicCoordinates& GraphicDevice::coordinates() const
{
	return m_settings.coordinates;
}

void GraphicDevice::frame()
{
	m_backbuffer->frame();
}

void GraphicDevice::present()
{
	m_backbuffer->submit();
}

Backbuffer::Ptr GraphicDevice::backbuffer()
{
	return m_backbuffer;
}

};