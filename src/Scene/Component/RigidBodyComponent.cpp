#include <Aka/Scene/Component/RigidBodyComponent.hpp>
#include <Aka/Renderer/Renderer.hpp>
#include <Aka/OS/Stream/MemoryStream.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveRigidBodyComponent::ArchiveRigidBodyComponent(ArchiveComponentVersionType _version) :
	ArchiveComponent(Component<RigidBodyComponent, ArchiveRigidBodyComponent>::getComponentID(), _version)
{
}

void ArchiveRigidBodyComponent::parse(BinaryArchive& archive)
{
	//archive.parse<AssetID>(assetID);
}

RigidBodyComponent::RigidBodyComponent(Node* node) :
	Component(node),
	m_velocity(0.f)
{
}
RigidBodyComponent::~RigidBodyComponent()
{
}
void RigidBodyComponent::onBecomeActive(AssetLibrary* library, Renderer* _renderer)
{
}
void RigidBodyComponent::onBecomeInactive(AssetLibrary* library, Renderer* _renderer)
{
}
void RigidBodyComponent::onRenderUpdate(AssetLibrary* library, Renderer* _renderer)
{
}
void RigidBodyComponent::onUpdate(Time deltaTime)
{
	// TODO fixed update here ?
	const float mass = 1.f; // kg (f=ma)
	const vec3f gravity(0.f, -9.81f, 0.f);
	const vec3f airResistance(0.f, 2.f, 0.f);
	vec3f forces(0.f);
	forces += gravity;
	forces += airResistance;
	vec3f acceleration = forces / mass; // F=ma, acceleration is in m/s^2
	m_velocity += acceleration * deltaTime.seconds();
	// TODO: should use some curve that convert raw velocity to limited value (log is too harsh)
	//const float maxVelocity = 256.f; // m/s
	//const float velocityScale = 5.f; // m/s
	//(geometry::log2(m_velocity.x) + maxVelocity);
	getNode()->setLocalTransform(mat4f::translate(getNode()->getLocalTransform(), m_velocity * deltaTime.seconds()));
}
void RigidBodyComponent::fromArchive(const ArchiveRigidBodyComponent& archive)
{
}

void RigidBodyComponent::toArchive(ArchiveRigidBodyComponent& archive)
{
}

};