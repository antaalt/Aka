#include <Aka/Graphic/Device.h>

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Core/Application.h>

namespace aka {

Device Device::getDefault()
{
	return Application::graphic()->getDevice(0);
}

Device Device::get(uint32_t id)
{
	return Application::graphic()->getDevice(id);
}

uint32_t Device::count()
{
	return Application::graphic()->getDeviceCount();
}

};