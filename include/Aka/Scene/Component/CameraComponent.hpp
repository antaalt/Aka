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
	Virtual, // Not controllable
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
	// A controller is linked to a window.
	CameraController(PlatformWindow* window) : m_window(window) {}
	virtual ~CameraController() {}

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

	// Get underlying window
	PlatformWindow* getWindow() const { return m_window; }
	// Set underlying window
	void setWindow(PlatformWindow* window) { m_window = window; }
private:
	PlatformWindow* m_window;
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
	CameraArcball(PlatformWindow* window) : CameraController(window) {}

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
	ArchiveCameraComponent(ArchiveComponentVersionType _version);

	void parse(BinaryArchive& archive) override;

	CameraProjectionType projectionType;
};

class CameraComponent : public Component<CameraComponent, ArchiveCameraComponent>
{
public:
	CameraComponent(Node* node);
	~CameraComponent();

	// Get camera projection data
	CameraProjection* getProjection() { return m_projection; }
	// Get camera projection data
	const CameraProjection* getProjection() const { return m_projection; }

public:
	// Get the view matrix 
	mat4f getViewMatrix() const;
	// Get the projection matrix
	mat4f getProjectionMatrix() const;
	// Set near plane for camera
	void setNear(float near);
	// Set far plane for camera
	void setFar(float far);
public:
	void onBecomeActive(AssetLibrary* library, Renderer* _renderer) override;
	void onBecomeInactive(AssetLibrary* library, Renderer* _renderer) override;
	void onUpdate(Time deltaTime) override;
	void onRenderUpdate(AssetLibrary* library, Renderer* _renderer) override;
public:
	void fromArchive(const ArchiveCameraComponent& archive) override;
	void toArchive(ArchiveCameraComponent& archive) override;

private:
	ViewHandle m_view;
	CameraProjection* m_projection;
};

AKA_DECL_COMPONENT(CameraComponent);


};