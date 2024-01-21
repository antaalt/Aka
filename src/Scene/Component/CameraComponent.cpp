#include <Aka/Scene/Component/CameraComponent.hpp>

#include <Aka/Platform/Input.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Core/Application.h>

namespace aka {

mat4f CameraPerspective::projection() const
{
	return mat4f::perspective(hFov, ratio, nearZ, farZ);
}

CameraProjectionType CameraPerspective::type() const
{
	return CameraProjectionType::Perpective;
}
void CameraPerspective::setViewport(uint32_t width, uint32_t height)
{
	ratio = width / (float)height;
}
void CameraPerspective::setNear(float near)
{
	this->nearZ = near;
}
void CameraPerspective::setFar(float far)
{
	this->farZ = far;
}

mat4f CameraOrthographic::projection() const
{
	return mat4f::orthographic(bottom, top, left, right, nearZ, farZ);
}

CameraProjectionType CameraOrthographic::type() const
{
	return CameraProjectionType::Orthographic;
}
void CameraOrthographic::setViewport(uint32_t width, uint32_t height)
{
	// TODO
}
void CameraOrthographic::setNear(float near)
{
	this->nearZ = near;
}
void CameraOrthographic::setFar(float far)
{
	this->farZ = far;
}

bool CameraArcball::update(Time deltaTime)
{
	Application* app = Application::app();
	PlatformDevice* platform = app->platform();
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
		if (dist - coeff > 0.1f)
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

CameraComponent::CameraComponent(Node* node) :
	Component(node),
	m_view(ViewHandle::Invalid),
	m_projection(nullptr)
{
}
CameraComponent::~CameraComponent()
{
	delete m_projection;
}

void ArchiveCameraComponent::parse(BinaryArchive& archive)
{
	archive.parse<CameraProjectionType>(projectionType);
}
mat4f CameraComponent::getViewMatrix() const 
{
	// TODO: should cache this heavy compute inverse
	return mat4f::inverse(getNode()->getWorldTransform()); 
}
mat4f CameraComponent::getProjectionMatrix() const 
{
	// TODO should cache this aswell
	return m_projection->projection();
}
void CameraComponent::setNear(float near)
{
	m_projection->setNear(near);
}
void CameraComponent::setFar(float far)
{
	m_projection->setFar(far);
}
void CameraComponent::onBecomeActive(AssetLibrary* library, Renderer* _renderer)
{
	m_view = _renderer->createView(ViewType::Color);
	getNode()->setLocalTransform(getNode()->getLocalTransform()); // Mark dirty to send on GPU
}
void CameraComponent::onBecomeInactive(AssetLibrary* library, Renderer* _renderer)
{
	_renderer->destroyView(m_view);
}
void CameraComponent::onUpdate(Time deltaTime)
{
}
void CameraComponent::onRenderUpdate(AssetLibrary* library, Renderer* _renderer)
{
	m_projection->setViewport(_renderer->getWidth(), _renderer->getHeight());
	// TODO should set if is visible (is MainCamera) in order to avoid computing it every frame.
	if (getNode()->has(NodeUpdateFlag::TransformUpdated) || isDirty())
	{
		mat4f view = mat4f::inverse(getNode()->getWorldTransform());
		_renderer->updateView(
			m_view,
			view,
			getProjectionMatrix()
		);
		clearDirty();
	}
}
void CameraComponent::fromArchive(const ArchiveCameraComponent& archive)
{
	AKA_ASSERT(m_projection == nullptr, "Projection not null");
	switch (archive.projectionType)
	{
	case CameraProjectionType::Orthographic:
		m_projection = new CameraOrthographic;
		break;
	case CameraProjectionType::Perpective:
		m_projection = new CameraPerspective;
		break;
	default:
		AKA_UNREACHABLE;
		break;
	}
}

void CameraComponent::toArchive(ArchiveCameraComponent& archive)
{
	archive.projectionType = m_projection->type();
}

};
