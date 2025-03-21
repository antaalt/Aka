#include <Aka/Graphic/Surface.h>
#include <Aka/PLatform/PlatformWindow.h>

namespace aka {
namespace gfx {

Surface::Surface(const char* name, PlatformWindow* window) :
	Resource(name, ResourceType::Surface),
	window(window)
{
}

}
}