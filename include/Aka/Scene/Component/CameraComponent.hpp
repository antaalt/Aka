#pragma once

#include <Aka/Core/Geometry.h>

#include <Aka/OS/Time.h>
#include <Aka/Resource/Archive/ArchiveScene.hpp>
#include <Aka/Renderer/View.hpp>
#include <Aka/Scene/ECS/World.hpp>

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
	// Pass viewport information to the camera.
	virtual void setViewport(uint32_t width, uint32_t height) = 0;
	virtual void setNear(float near) = 0;
	virtual void setFar(float far) = 0;
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
	void setViewport(uint32_t width, uint32_t height) override;
	void setNear(float near) override;
	void setFar(float far) override;
	
	anglef hFov = anglef::degree(60.f);
	float ratio = 1.f;
	float nearZ = 0.1f;
	float farZ  = 100.f;
};

struct CameraOrthographic : CameraProjection
{
	mat4f projection() const override;
	CameraProjectionType type() const override;
	void setViewport(uint32_t width, uint32_t height) override;
	void setNear(float near) override;
	void setFar(float far) override;

	float left   = -1.f;
	float right  =  1.f;
	float bottom = -1.f;
	float top	 =  1.f;
	float nearZ = 0.1f;
	float farZ  = 100.f;
};


struct CameraArcball : CameraController
{
	bool update(Time deltaTime) override;
	mat4f transform() const override;
	mat4f view() const override;
	CameraControllerType type() const override;
	void set(const aabbox<>& bbox) override;

	point3f position = point3f(0.f);
	point3f target;
	norm3f up;

	float speed;
};

// -----------------------------------------

struct CameraComponent
{
	CameraProjection* projection;
	ViewHandle viewHandle;

	void setNear(float near) { projection->setNear(near); }
	void setFar(float far) { projection->setFar(far); }
};
AKA_DECL_COMPONENT(CameraComponent);

struct ecs::ArchiveComponent<CameraComponent>
{
	CameraProjectionType type;
	void from(const CameraComponent& component)
	{
		type = component.projection->type();
	}
	void to(CameraComponent& component) const
	{
		AKA_ASSERT(component.projection == nullptr, "Projection not null. Leak.");
		switch (type)
		{
		case CameraProjectionType::Orthographic:
			component.projection = new CameraOrthographic;
			break;
		case CameraProjectionType::Perpective:
			component.projection = new CameraPerspective;
			break;
		default:
			AKA_NOT_IMPLEMENTED;
			break;
		}
	}
	void parse(BinaryArchive& archive)
	{
		archive.parse(type);
	}
};

};