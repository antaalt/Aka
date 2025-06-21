#pragma once

#include <Aka/Core/Enum.h>
#include <Aka/Core/Geometry.h>
#include <Aka/Graphic/GraphicDevice.h>

namespace aka {

// namespace rdr {
// class Viewport; // Hold informations about which swapchain / window & section to render. Can be headless.
// class View; // Hold informations about camera & visibility.
// class Memory; // Hold buffer & ease the process of uploading data to memory.
// class Renderer; // Hold informations about rendering generic geometry types. Can be specialized (Static, Skeletal...)
// class Request; // Async request for renderer (CreateMesh, DestroyMesh, InstantiateMesh...)
// }
// This is a renderer viewport.
// Any viewport can contain multiple views.
// Each layer should have a viewport.
// Each window should have one too.
class Viewport {
public:
private:
	gfx::SwapchainHandle m_swapchain;
};

}