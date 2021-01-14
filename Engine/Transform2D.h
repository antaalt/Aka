#pragma once

#include "Component.h"
#include "Geometry.h"

namespace app {

struct Transform2D : public Component
{
	Transform2D();
	Transform2D(const vec2f& position, const vec2f& size, radianf rotation);

	mat4f model() const;

	vec2f position;
	vec2f size;
	radianf rotation;
};

}