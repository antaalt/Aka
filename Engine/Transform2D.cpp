#include "Transform2D.h"

namespace aka {
Transform2D::Transform2D() :
	Transform2D(vec2f(0.f), vec2f(1.f), radianf(0.f))
{
}
Transform2D::Transform2D(const vec2f& position, const vec2f& size, radianf rotation) :
	position(position),
	size(size),
	rotation(rotation)
{
}

mat4f Transform2D::model() const
{
	mat4f model = mat4f::identity();
	// Translate
	model *= mat4f::translate(vec3f(position, 0.f));
	// Rotate around center of object
	model *= mat4f::translate(vec3f(0.5f * size.x, 0.5f * size.y, 0.0f));
	model *= mat4f::rotate(vec3f(0.0f, 0.0f, 1.0f), rotation);
	model *= mat4f::translate(vec3f(-0.5f * size.x, -0.5f * size.y, 0.0f));
	// Scale
	model *= mat4f::scale(vec3f(size, 1.0f));
	return model;
}

};