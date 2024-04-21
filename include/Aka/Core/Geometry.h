#pragma once

// D3D / Metal / Consoles coordinate system convention left handed
// OpenGL / OpenGL ES coordinate system convention is right handed
// Aka use top right handed as default for everything because its easier.
#if !defined(GEOMETRY_RIGHT_HANDED) && !defined(GEOMETRY_LEFT_HANDED)
#define GEOMETRY_RIGHT_HANDED
#endif

// Clip space range differ from GL & D3D11. 
// This affect the perspective matrix for clip space transformation.
#if defined(AKA_USE_OPENGL)
#define GEOMETRY_CLIP_SPACE_NEGATIVE // [-1, 1]
#elif defined(AKA_USE_D3D11) || defined(AKA_USE_VULKAN)
#define GEOMETRY_CLIP_SPACE_POSITIVE // [0, 1]
#else
#error "Undetected API"
#endif

// No need even though we are using vulkan as we work it around with viewport flip. 
// Check VulkanGraphicPipeline::createVkGraphicPipeline
//#define GEOMETRY_REVERSED_Y

#include <geo/geometry.h>

namespace aka {
using namespace geometry;
}