#include <Aka/Graphic/GraphicDevice.h>

namespace aka {

GraphicDevice::GraphicDevice(const GraphicConfig& config) :
	m_features{},
	m_backbuffer(nullptr)
{
}

GraphicDevice::~GraphicDevice()
{
}

const GraphicApi& GraphicDevice::api() const
{
	return m_features.api;
}

const GraphicDeviceFeatures& GraphicDevice::features() const
{
	return m_features;
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