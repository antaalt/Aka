#include "Device.h"

#include "GraphicBackend.h"

namespace aka {

Device Device::getDefault()
{
	return GraphicBackend::getDevice(0);
}

Device Device::get(uint32_t id)
{
	return GraphicBackend::getDevice(id);
}

uint32_t Device::count()
{
	return GraphicBackend::deviceCount();
}

};