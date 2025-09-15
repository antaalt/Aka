#include <Aka/Graphic/Surface.h>
#include <Aka/Platform/PlatformWindow.h>

namespace aka {
namespace gfx {

Surface::Surface(const char* name, PlatformWindow* window) :
	Resource(name, ResourceType::Surface),
	window(window)
{
}

}
}