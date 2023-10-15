#pragma once

#include <Aka/Core/Geometry.h>

#include <Aka/OS/Time.h>
#include <Aka/Scene/Component.hpp>
#include <Aka/Resource/Archive/ArchiveScene.hpp>
#include <Aka/Renderer/View.hpp>

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


struct ArchiveCameraComponent : ArchiveComponent
{
	ArchiveCameraComponent();

	void parse(BinaryArchive& archive) override;

	CameraControllerType controllerType;
	CameraProjectionType projectionType;
};

class CameraComponent : public Component
{
public:
	CameraComponent(Node* node);
	~CameraComponent();

	// Get camera projection data
	CameraProjection* getProjection() { return m_projection; }
	// Get camera projection data
	const CameraProjection* getProjection() const { return m_projection; }
	// Get camera controller data
	CameraController* getController() { return m_controller; }
	// Get camera controller data
	const CameraController* getController() const { return m_controller; }

public:
	// Get the view matrix 
	mat4f getViewMatrix() const;
	// Get the projection matrix
	mat4f getProjectionMatrix() const;
	// Set if updates are enabled on this camera.
	void setUpdateEnabled(bool value);
	// Check if updates are enabled on this camera.
	bool isUpdateEnabled() const;
public:
	void onBecomeActive(AssetLibrary* library, Renderer* _renderer) override;
	void onBecomeInactive(AssetLibrary* library, Renderer* _renderer) override;
	void onUpdate(Time deltaTime) override;
	void onRenderUpdate(AssetLibrary* library, Renderer* _renderer) override;
public:
	void fromArchive(const ArchiveComponent& archive) override;
	void toArchive(ArchiveComponent& archive) override;

private:
	bool m_updateEnabled;
	ViewHandle m_view;
	CameraController* m_controller;
	CameraProjection* m_projection;
};

AKA_DECL_COMPONENT(CameraComponent);


};