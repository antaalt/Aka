#pragma once

#include "Geometry.h"
#include "Component.h"

namespace aka {

struct Camera2D : Component {
	Camera2D();
	Camera2D(const vec2f position, const vec2f& viewport);

	vec2f position;
	vec2f viewport;
};

}

