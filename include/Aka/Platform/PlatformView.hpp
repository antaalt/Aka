#pragma once

#include <Aka/Scene/Component/CameraComponent.hpp>

namespace aka {

struct VirtualCameraController : CameraController
{
	// Not linked to a window
	VirtualCameraController() : CameraController(nullptr) {}
	VirtualCameraController(PlatformWindow* window) : CameraController(window) {}

	bool update(Time deltaTime) override 
	{ 
		// Cannot be updated as virtual.
		return false; 
	}
	mat4f transform() const override
	{
		return m_transform;
	}
	mat4f view() const override
	{
		return mat4f::inverse(m_transform);
	}
	CameraControllerType type() const override
	{
		return CameraControllerType::Virtual;
	}
	void set(const aabbox<>& bbox) override
	{
		m_transform = mat4f::lookAt(bbox.center() + bbox.extent(), bbox.center());
	}
	void setTransform(const mat4f& transform)
	{
		m_transform = transform;
	}
private:
	mat4f m_transform;
};

class PlatformView {
public:
	// Create a headless view
	PlatformView(CameraProjection* projection);
	// Create a window view with a camera controller
	PlatformView(CameraController* controller, CameraProjection* projection);
	~PlatformView();

	// Update the camera controller
	bool update(Time deltaTime);

	// Get controller of camera. null if not a window view.
	CameraController* getController() const;
	// Get projection of camera
	CameraProjection* getProjection() const;
private:
	CameraController* m_controller = nullptr;
	CameraProjection* m_projection = nullptr;
};

}