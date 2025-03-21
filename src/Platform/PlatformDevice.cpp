#include <Aka/Platform/PlatformDevice.h>

#include "GLFW3/PlatformGLFW3.h"
#include <Aka/Core/Application.h>

namespace aka {

PlatformDevice* PlatformDevice::create()
{
#if defined(AKA_USE_GLFW3)
	return mem::akaNew<PlatformGLFW3>(AllocatorMemoryType::Object, AllocatorCategory::Global);
#else
	Logger::critical("No platform defined.");
	return nullptr;
#endif
}

void PlatformDevice::destroy(PlatformDevice* device)
{
	mem::akaDelete(device);
}

PlatformDevice::PlatformDevice()
{
}

PlatformDevice::~PlatformDevice()
{
}


};