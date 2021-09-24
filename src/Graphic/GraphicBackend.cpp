#include <Aka/Graphic/GraphicBackend.h>

#include "GL/GLDevice.h"
#include "D3D11/D3D11Device.h"

namespace aka {

static GraphicDevice* s_device = nullptr;

void GraphicBackend::initialize(uint32_t width, uint32_t height)
{
	if (s_device == nullptr)
	{
#if defined(AKA_USE_OPENGL)
		s_device = new GLDevice(width, height);
#elif defined(AKA_USE_D3D11)
		s_device = new D3D11Device(width, height);
#endif
	}
}

void GraphicBackend::destroy()
{
	if (s_device)
	{
		delete s_device;
		s_device = nullptr;
	}
}

GraphicDevice* GraphicBackend::device()
{
	return s_device;
}


};
