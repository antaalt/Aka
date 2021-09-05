#pragma once

#include <Aka/Core/Geometry.h>

#include <Aka/OS/Time.h>

namespace aka {

struct CameraProjection
{
	// Get the projection matrix
	virtual mat4f projection() const = 0;
};

struct CameraController
{
	// Update the camera using delta time
	virtual bool update(Time::Unit deltaTime) = 0;
	// Get the transform of the controller
	virtual mat4f transform() const = 0;
	// Get the view of the controller
	virtual mat4f view() const = 0;

	// Set the camera to look for a bounding box
	virtual void set(const aabbox<>& bbox) = 0;
};

struct CameraPerspective : CameraProjection
{
	mat4f projection() const override;
	
	anglef hFov;
	float ratio;
	float nearZ, farZ;
};

struct CameraOrthographic : CameraProjection
{
	mat4f projection() const override;

	vec2f viewport;
};


struct CameraArcball : CameraController
{
	bool update(Time::Unit deltaTime) override;
	mat4f transform() const override;
	mat4f view() const override;
	void set(const aabbox<>& bbox) override;

	point3f position;
	point3f target;
	norm3f up;

	float speed;
};

};