#pragma once

#include "../Core/Component.h"
#include "../Core/Geometry.h"

namespace aka {

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