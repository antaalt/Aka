#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Graphic/PhysicalDevice.h>

namespace aka {
namespace gfx {

class AKA_NO_VTABLE Instance
{
public:
	Instance() {}
	virtual ~Instance() {}

	virtual void initialize() = 0;
	virtual void shutdown() = 0;

	// Surface
	virtual SurfaceHandle createSurface(const char* name, PlatformWindow* window) = 0;
	virtual const Surface* get(SurfaceHandle handle) = 0;
	virtual void destroy(SurfaceHandle surface) = 0;

	// Device
	virtual GraphicDevice* pick(PhysicalDeviceFeatures features, PlatformWindow* window) = 0;
	virtual void destroy(GraphicDevice* device) = 0;

	static Instance* create(GraphicAPI api);
	static void destroy(Instance* instance);
};

}
}