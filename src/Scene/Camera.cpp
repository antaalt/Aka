#include <Aka/Scene/Camera.h>

namespace aka {

CameraPerspective::CameraPerspective(const vec2f& viewport, anglef hFov, float nearZ, float farZ) :
	viewport(viewport),
	hFov(hFov),
	nearZ(nearZ),
	farZ(farZ)
{
}

mat4f CameraPerspective::perspective() const
{
	return mat4f::perspective(hFov, viewport.x / viewport.y, nearZ, farZ);
}

CameraOrthographic::CameraOrthographic(const vec2f& viewport) :
	viewport(viewport)
{
}

mat4f CameraOrthographic::perspective() const
{
	return mat4f::orthographic(0, viewport.y, 0, viewport.x);
}

};