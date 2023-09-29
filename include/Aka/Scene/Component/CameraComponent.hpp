#pragma once

#include <Aka/Core/Geometry.h>

#include <Aka/OS/Time.h>
#include <Aka/Scene/Component.hpp>
#include <Aka/Resource/Archive/ArchiveScene.hpp>

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
	
	anglef hFov = anglef::degree(60.f);
	float ratio = 1.f;
	float nearZ = 0.1f;
	float farZ  = 100.f;
};

struct CameraOrthographic : CameraProjection
{
	mat4f projection() const override;
	CameraProjectionType type() const override;

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

	point3f position;
	point3f target;
	norm3f up;

	float speed;
};


struct ArchiveCameraComponent : ArchiveComponent
{
	ArchiveCameraComponent();

	void load_internal(BinaryArchive& archive) override;
	void save_internal(BinaryArchive& archive) override;

	CameraControllerType controllerType;
	CameraProjectionType projectionType;
};

class CameraComponent : public Component
{
public:
	CameraComponent();
	~CameraComponent();

	CameraProjection* getProjection() { return m_projection; }
	const CameraProjection* getProjection() const { return m_projection; }
	CameraController* getController() { return m_controller; }
	const CameraController* getController() const { return m_controller; }

public:
	void load(const ArchiveComponent& archive) override;
	void save(ArchiveComponent& archive) override;

private:
	CameraController* m_controller;
	CameraProjection* m_projection;
};

AKA_DECL_COMPONENT(CameraComponent);


};