#pragma once

#include <stdint.h>

#include <Aka/Graphic/Resource.h>
#include <Aka/Core/Enum.h>

namespace aka {

class PlatformWindow;

namespace gfx {

struct Surface;
using SurfaceHandle = ResourceHandle<Surface>;

struct Surface : Resource
{
	Surface(const char* name, PlatformWindow* window);
	virtual ~Surface() {}

	PlatformWindow* window; // Underlying window of the surface
};

};
};