#pragma once

#include <Aka/Core/Geometry.h>

#include <Aka/OS/Time.h>
#include <Aka/Scene/Component.hpp>

namespace aka {

enum class CameraProjectionType
{
	Perpective,
	Orthographic
};

enum class CameraControllerType
{
	Arcball
};

struct CameraProjection
{
	// Get the projection matrix
	virtual mat4f projection() const = 0;
	// Get the projection type
	virtual CameraProjectionType type() const = 0;
};

struct CameraController
{
	// Update the camera using delta time
	virtual bool update(Time deltaTime) = 0;
	// Get the transform of the controller
	virtual mat4f transform() const = 0;
	// Get the view of the controller
	virtual mat4f view() const = 0;
	// Get the controller type
	virtual CameraControllerType type() const = 0;

	// Set the camera to look for a bounding box
	virtual void set(const aabbox<>& bbox) = 0;
};

struct CameraPerspective : CameraProjection
{
	mat4f projection() const override;
	CameraProjectionType type() const override;
	
	anglef hFov;
	float ratio;
	float nearZ, farZ;
};

struct CameraOrthographic : CameraProjection
{
	mat4f projection() const override;
	CameraProjectionType type() const override;

	float left, right, bottom, top;
	float nearZ, farZ;
};


struct CameraArcball : CameraController
{
	bool update(Time deltaTime) override;
	mat4f transform() const override;
	mat4f view() const override;
	CameraControllerType type() const override;
	void set(const aabbox<>& bbox) override;

	point3f position;
	point3f target;
	norm3f up;

	float speed;
};

class CameraComponent : public Component
{
public:
	CameraComponent();

private:
	CameraController* m_controller;
	CameraProjection* m_projection;
};


};