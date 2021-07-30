#include <Aka/Scene/Camera.h>

namespace aka {

CameraPerspective::CameraPerspective(anglef hFov, float ratio, float nearZ, float farZ) :
	hFov(hFov),
	ratio(ratio),
	nearZ(nearZ),
	farZ(farZ)
{
}

mat4f CameraPerspective::projection() const
{
	return mat4f::perspective(hFov, ratio, nearZ, farZ);
}

CameraOrthographic::CameraOrthographic(const vec2f& viewport) :
	viewport(viewport)
{
}

mat4f CameraOrthographic::projection() const
{
	return mat4f::orthographic(0, viewport.y, 0, viewport.x);
}

};