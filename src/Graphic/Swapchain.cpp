#include <Aka/Graphic/Swapchain.h>

#include <Aka/Core/Application.h>

#include <type_traits>

namespace aka {
namespace gfx {

Swapchain::Swapchain(const char* name, SurfaceHandle surface, uint32_t width, uint32_t height, TextureFormat format, SwapchainMode mode, SwapchainType type) :
	Resource(name, ResourceType::Swapchain),
	width(width),
	height(height),
	format(format),
	mode(mode),
	type(type),
	imageCount(0),
	surface(surface)
{

}
}
}