#include <Aka/Scene/Component/ColliderComponent.hpp>
#include <Aka/Scene/Component/RigidBodyComponent.hpp>
#include <Aka/Renderer/Renderer.hpp>
#include <Aka/OS/Stream/MemoryStream.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

bool ContactData::isColliding() const
{
	AKA_ASSERT(node1 != node2, "Self intersection.");
	AKA_ASSERT(node1->has<RigidBodyComponent>(), "Node1 is not dynamic.");
	AKA_ASSERT(node1->has<ColliderComponent>(), "Node1 is not a collider.");
	AKA_ASSERT(node2->has<ColliderComponent>(), "Node2 is not a collider.");
	return penetration < 0.f;
}

ArchiveColliderComponent::ArchiveColliderComponent(ArchiveComponentVersionType _version) :
	ArchiveComponent(Component<ColliderComponent, ArchiveColliderComponent>::getComponentID(), _version)
{
}

void ArchiveColliderComponent::parse(BinaryArchive& archive)
{
	archive.parse<ColliderShapeType>(shape);
}

ColliderComponent::ColliderComponent(Node* node) :
	Component(node),
	m_shape(nullptr)
{
}
ColliderComponent::~ColliderComponent()
{
	mem::akaDelete(m_shape);
}
void ColliderComponent::onBecomeActive(AssetLibrary* library, Renderer* _renderer)
{
}
void ColliderComponent::onBecomeInactive(AssetLibrary* library, Renderer* _renderer)
{
}
void ColliderComponent::onRenderUpdate(AssetLibrary* library, Renderer* _renderer)
{
}
void ColliderComponent::onUpdate(Time deltaTime)
{
}
ContactData computeCollisionSphereToSphere(Node* node1, Node* node2, const point3f& position1, float radius1, const point3f& position2, float radius2)
{
	vec3f direction = position2 - position1;
	float distance = vec3f::length(direction);
	direction = distance == 0.f ? vec3f::up() : direction / distance;
	ContactData contact;
	contact.node1 = node1;
	contact.node2 = node2;
	contact.surfaceHit1 = position1 + direction * radius1;
	contact.surfaceHit2 = position2 - direction * radius2;
	contact.normal1 = direction;
	contact.normal2 = -contact.normal1;
	contact.penetration = distance - radius1 - radius2;
	return contact;
}

ContactData computeCollisionSphereToPlane(Node* nodeSphere, Node* nodePlane, const point3f& spherePosition, float sphereRadius, const point3f& pointOnPlane, const vec3f& planeNormal)
{
	vec3f direction = -planeNormal;
	float distance = vec3f::dot(vec3f(planeNormal), spherePosition - pointOnPlane);
	ContactData contact;
	contact.node1 = nodeSphere;
	contact.node2 = nodePlane;
	contact.surfaceHit1 = spherePosition + vec3f(direction * sphereRadius);
	contact.surfaceHit2 = spherePosition + vec3f(direction * distance);
	contact.normal1 = vec3f::normalize(direction);
	contact.normal2 = planeNormal;
	contact.penetration = distance - sphereRadius;
	return contact;
}
ContactData computeCollisionPlaneToSphere(Node* nodePlane, Node* nodeSphere, const point3f& pointOnPlane, const vec3f& planeNormal, const point3f& spherePosition, float sphereRadius)
{
	ContactData contact = computeCollisionSphereToPlane(nodeSphere, nodePlane, spherePosition, sphereRadius, pointOnPlane, planeNormal);
	std::swap(contact.node1, contact.node2);
	std::swap(contact.surfaceHit1, contact.surfaceHit2);
	std::swap(contact.normal1, contact.normal2);
	return contact;
}
bool ColliderComponent::isColliding(ColliderComponent& _other, ContactData& _contact)
{
	switch (m_shape->getShapeType())
	{
	case ColliderShapeType::Cube:
		AKA_NOT_IMPLEMENTED;
		Logger::error("No collision detection yet for cube");
		return false;
	case ColliderShapeType::Sphere: {
		const ColliderSphere* collider1 = reinterpret_cast<const ColliderSphere*>(m_shape);
		switch (_other.m_shape->getShapeType())
		{
		case ColliderShapeType::Cube:
			AKA_NOT_IMPLEMENTED;
			Logger::error("No collision detection yet for Sphere / Cube");
			return false;
		case ColliderShapeType::Sphere: {
			const ColliderSphere* collider2 = reinterpret_cast<const ColliderSphere*>(_other.m_shape);
			// TODO: might not need transforming as we expect local space, but might need to rotate though
			_contact = computeCollisionSphereToSphere(
				getNode(),
				_other.getNode(),
				getNode()->getWorldTransform().multiplyPoint(collider1->m_offset),
				collider1->m_radius, // TODO: scale
				_other.getNode()->getWorldTransform().multiplyPoint(collider2->m_offset),
				collider2->m_radius // TODO: scale
			);
			return _contact.isColliding();
		}
		case ColliderShapeType::Plane: {
			const ColliderPlane* collider2 = reinterpret_cast<const ColliderPlane*>(_other.m_shape);
			_contact = computeCollisionSphereToPlane(
				getNode(),
				_other.getNode(),
				getNode()->getWorldTransform().multiplyPoint(collider1->m_offset),
				collider1->m_radius, // TODO: scale
				_other.getNode()->getWorldTransform().multiplyPoint(collider2->m_offset),
				vec3f::normalize(_other.getNode()->getWorldTransform().multiplyVector(collider2->m_normal))
			);
			return _contact.isColliding();
		}
		default:
			AKA_UNREACHABLE;
			return false;
		}
		break;
	}
	case ColliderShapeType::Plane: {
		const ColliderPlane* collider1 = reinterpret_cast<const ColliderPlane*>(m_shape);
		switch (_other.m_shape->getShapeType())
		{
		case ColliderShapeType::Cube:
			AKA_NOT_IMPLEMENTED;
			Logger::error("No collision detection yet for Plane / Cube");
			return false;
		case ColliderShapeType::Sphere: {
			const ColliderSphere* collider2 = reinterpret_cast<const ColliderSphere*>(_other.m_shape);
			// TODO: might not need transforming as we expect local space, but might need to rotate though
			_contact = computeCollisionPlaneToSphere(
				getNode(),
				_other.getNode(),
				getNode()->getWorldTransform().multiplyPoint(collider1->m_offset),
				vec3f::normalize(_other.getNode()->getWorldTransform().multiplyVector(collider1->m_normal)),
				_other.getNode()->getWorldTransform().multiplyPoint(collider2->m_offset),
				collider2->m_radius
			);
			return _contact.isColliding();
		}
		case ColliderShapeType::Plane:
			AKA_NOT_IMPLEMENTED;
			Logger::error("No collision detection yet for Plane / Plane");
			return false;
		default:
			AKA_UNREACHABLE;
			return false;
		}
		return false;
	}
	default:
		AKA_UNREACHABLE;
		return false;
	}
}
void ColliderComponent::fromArchive(const ArchiveColliderComponent& archive)
{
	AKA_ASSERT(m_shape == nullptr, "");
	switch (archive.shape) {
	case ColliderShapeType::Cube:
		m_shape = mem::akaNew<ColliderCube>(AllocatorMemoryType::Object, AllocatorCategory::Global);
		break;
	case ColliderShapeType::Sphere:
		m_shape = mem::akaNew<ColliderSphere>(AllocatorMemoryType::Object, AllocatorCategory::Global);
		break;
	case ColliderShapeType::Plane:
		m_shape = mem::akaNew<ColliderPlane>(AllocatorMemoryType::Object, AllocatorCategory::Global);
		break;
	default:
		AKA_NOT_IMPLEMENTED;
		break;
	}
}
void ColliderComponent::toArchive(ArchiveColliderComponent& archive)
{
	archive.shape = m_shape->getShapeType();
}

};