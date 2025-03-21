#pragma once

#include <stdint.h>

#include <Aka/Graphic/Resource.h>
#include <Aka/Graphic/Surface.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Core/Enum.h>

namespace aka {
namespace gfx {

enum class SwapchainType {
	Performance, // Priorize rendering speed.
	Latency, // Priorize reduced latency.
};
enum class SwapchainMode {
	Windowed,
	Borderless,
	Fullscreen,
};
struct SwapchainExtent {
	uint32_t width;
	uint32_t height;
};

// GLFW does not support HDR yet.
// https://github.com/glfw/glfw/issues/890

struct Swapchain;
using SwapchainHandle = ResourceHandle<Swapchain>;

struct Swapchain : Resource
{
	Swapchain(const char* name, SurfaceHandle surface, uint32_t width, uint32_t height, TextureFormat format, SwapchainMode mode, SwapchainType type);
	virtual ~Swapchain() {}

	uint32_t width; // Width of the swapchain
	uint32_t height; // Height of the swapchain
	TextureFormat format; // Underlying format of the swapchain
	uint32_t imageCount;
	SwapchainMode mode;
	SwapchainType type;
	SurfaceHandle surface;
};

};
};