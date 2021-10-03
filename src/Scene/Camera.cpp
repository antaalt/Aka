#include <Aka/Scene/Camera.h>

#include <Aka/Platform/Input.h>
#include <Aka/Platform/PlatformDevice.h>

namespace aka {

mat4f CameraPerspective::projection() const
{
	return mat4f::perspective(hFov, ratio, nearZ, farZ);
}

CameraProjectionType CameraPerspective::type() const
{
	return CameraProjectionType::Perpective;
}

mat4f CameraOrthographic::projection() const
{
	return mat4f::orthographic(bottom, top, left, right, nearZ, farZ);
}

CameraProjectionType CameraOrthographic::type() const
{
	return CameraProjectionType::Orthographic;
}

bool CameraArcball::update(Time deltaTime)
{
	PlatformDevice* platform = PlatformBackend::get();
	const Mouse& mouse = platform->mouse();
	bool dirty = false;
	// https://gamedev.stackexchange.com/questions/53333/how-to-implement-a-basic-arcball-camera-in-opengl-with-glm
	if (mouse.pressed(MouseButton::ButtonLeft) && (mouse.delta().x != 0.f || mouse.delta().y != 0.f))
	{
		float x = mouse.delta().x * deltaTime.seconds();
		float y = -mouse.delta().y * deltaTime.seconds();
		anglef pitch = anglef::radian(y);
		anglef yaw = anglef::radian(x);
		vec3f upCamera = vec3f(0, 1, 0);
		vec3f forwardCamera = vec3f::normalize(target - position);
		vec3f rightCamera = vec3f::normalize(vec3f::cross(forwardCamera, vec3f(upCamera)));
		position = mat4f::rotate(rightCamera, pitch).multiplyPoint(point3f(position - target)) + vec3f(target);
		position = mat4f::rotate(upCamera, yaw).multiplyPoint(point3f(position - target)) + vec3f(target);
		dirty = true;
	}
	if (mouse.pressed(MouseButton::ButtonRight) && (mouse.delta().x != 0.f || mouse.delta().y != 0.f))
	{
		float x = -mouse.delta().x * deltaTime.seconds();
		float y = -mouse.delta().y * deltaTime.seconds();
		vec3f upCamera = vec3f(0, 1, 0); // TODO change it when close to up
		vec3f forwardCamera = vec3f::normalize(target - position);
		vec3f rightCamera = vec3f::normalize(vec3f::cross(forwardCamera, vec3f(upCamera)));
		vec3f move = rightCamera * x * speed / 2.f + upCamera * y * speed / 2.f;
		target += move;
		position += move;
		dirty = true;
	}
	if (mouse.scroll().y != 0.f)
	{
		float zoom = mouse.scroll().y * deltaTime.seconds();
		vec3f dir = vec3f::normalize(target - position);
		float dist = point3f::distance(target, position);
		float coeff = zoom * speed;
		if (dist - coeff > 1.5f)
		{
			position = position + dir * coeff;
			dirty = true;
		}
	}
	return dirty;
}

mat4f CameraArcball::transform() const
{
	return mat4f::lookAt(position, target, up);
}

void CameraArcball::set(const aabbox<>& bbox)
{
	float dist = bbox.extent().norm();
	position = bbox.max * 1.2f;
	target = bbox.center();
	up = norm3f(0, 1, 0);
	speed = dist;
}

mat4f CameraArcball::view() const
{
	return mat4f::lookAtView(position, target, up); 
}

CameraControllerType CameraArcball::type() const
{
	return CameraControllerType::Arcball;
}

};
