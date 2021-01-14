#include "Camera2D.h"

namespace aka {

Camera2D::Camera2D() :
	Camera2D(vec2f(0.f), vec2f(1.f))
{
}

Camera2D::Camera2D(const vec2f position, const vec2f& viewport) :
	position(position),
	viewport(viewport)
{
}

}