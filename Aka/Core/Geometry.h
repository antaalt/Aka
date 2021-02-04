#pragma once

#if defined(AKA_USE_OPENGL)
#define GEOMETRY_RIGHT_HANDED
#define GEOMETRY_CLIP_SPACE_NEGATIVE
#elif defined(AKA_USE_D3D11)
#define GEOMETRY_LEFT_HANDED
#define GEOMETRY_CLIP_SPACE_POSITIVE
#else
#endif

#include <geometry.h>

namespace aka {
using namespace geometry;
}