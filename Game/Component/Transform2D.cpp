#include "Transform2D.h"

namespace aka {
Transform2D::Transform2D() :
	model(mat3f::identity())
{
}
Transform2D::Transform2D(const vec2f& position, const vec2f& scale, radianf rotation) :
	model(mat3f::identity())
{
	model *= mat3f::translate(position);
	model *= mat3f::rotate(rotation);
	model *= mat3f::scale(scale);
}

vec2f Transform2D::position() const
{
	return vec2f(model[2].x, model[2].y);
}

vec2f Transform2D::size() const
{
	return vec2f(model[0].x, model[1].y);
}

void Transform2D::translate(const vec2f &translation)
{
	model *= mat3f::translate(translation);
}

void Transform2D::move(const vec2f& move)
{
	model[2].x += move.x;
	model[2].y += move.y;
}

void Transform2D::rotate(radianf rotation)
{
	model *= mat3f::rotate(rotation);
}

void Transform2D::scale(const vec2f &scale)
{
	model *= mat3f::scale(scale);
}

/*mat4f Transform2D::model() const
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
}*/

};