#pragma once

#include <Aka/Core/Geometry.h>

namespace aka {

struct CameraProjection
{
	virtual mat4f projection() const = 0;
};

struct CameraPerspective : CameraProjection
{
	CameraPerspective() {}
	CameraPerspective(anglef hFov, float ratio, float nearZ = 0.1f, float farZ = 100.f);

	mat4f projection() const override;
	
	anglef hFov;
	float ratio;
	float nearZ, farZ;
};

struct CameraOrthographic : CameraProjection
{
	CameraOrthographic() {}
	CameraOrthographic(const vec2f& viewport);

	mat4f projection() const override;

	vec2f viewport;
};

};