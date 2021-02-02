#pragma once

#include <Core/ECS/Component.h>
#include <Core/Geometry.h>

namespace aka {

struct Transform2D : public Component
{
	Transform2D();
	Transform2D(const vec2f& position, const vec2f& scale, radianf rotation);

	mat3f model;

	vec2f position() const;
	vec2f size() const;

	void translate(const vec2f& translation);
	void move(const vec2f& move);
	void rotate(radianf rotation);
	void scale(const vec2f& scale);
};

}