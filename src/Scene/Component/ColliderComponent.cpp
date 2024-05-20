#include <Aka/Scene/Component/ColliderComponent.hpp>
#include <Aka/Scene/Component/RigidBodyComponent.hpp>
#include <Aka/Renderer/Renderer.hpp>
#include <Aka/OS/Stream/MemoryStream.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

const ContactData ContactData::Invalid = {};

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
ContactData computeCollision(Node* node1, Node* node2, const ColliderSphere& sphere1, const ColliderSphere& sphere2)
{
	AKA_ASSERT(sphere1.getShapeType() == ColliderShapeType::Sphere, "Invalid shape");
	AKA_ASSERT(sphere2.getShapeType() == ColliderShapeType::Sphere, "Invalid shape");
	point3f worldPos1 = node1->getWorldTransform().multiplyPoint(sphere1.m_offset);
	point3f worldPos2 = node2->getWorldTransform().multiplyPoint(sphere2.m_offset);

	vec3f direction = worldPos2 - worldPos1;
	float distance = vec3f::length(direction);
	direction = distance == 0.f ? direction : direction / distance;
	ContactData contact;
	contact.node1 = node1;
	contact.node2 = node2;
	contact.surfaceHit1 = worldPos1 + direction * sphere1.m_radius;
	contact.surfaceHit2 = worldPos2 - direction * sphere2.m_radius;
	contact.normal1 = direction;
	contact.normal2 = -contact.normal1;
	contact.penetration = distance - sphere1.m_radius - sphere2.m_radius;
	return contact;
}

ContactData computeCollision(Node* nodeSphere, Node* nodePlane, const ColliderSphere& sphere, const ColliderPlane& plane)
{
	AKA_ASSERT(sphere.getShapeType() == ColliderShapeType::Sphere, "Invalid shape");
	AKA_ASSERT(plane.getShapeType() == ColliderShapeType::Plane, "Invalid shape");
	point3f worldPosSphere = nodeSphere->getWorldTransform().multiplyPoint(sphere.m_offset);
	point3f worldPosPlane = nodePlane->getWorldTransform().multiplyPoint(plane.m_offset);
	vec3f worldRadiusSphere = mat4f::extractScale(nodeSphere->getWorldTransform());
	vec3f worldNormalPlane = vec3f::normalize(nodePlane->getWorldTransform().multiplyVector(plane.m_normal));

	vec3f direction = -worldNormalPlane;
	float distance = vec3f::dot(vec3f(worldNormalPlane), worldPosSphere - worldPosPlane);
	ContactData contact;
	contact.node1 = nodeSphere;
	contact.node2 = nodePlane;
	contact.surfaceHit1 = worldPosSphere + direction * worldRadiusSphere;
	contact.surfaceHit2 = worldPosSphere + vec3f(direction * distance);
	contact.normal1 = vec3f::normalize(direction);
	contact.normal2 = worldNormalPlane;
	contact.penetration = distance - sphere.m_radius;
	return contact;
}
ContactData computeCollision(Node* nodePlane, Node* nodeSphere, const ColliderPlane& plane, const ColliderSphere& sphere)
{
	ContactData contact = computeCollision(nodeSphere, nodePlane, sphere, plane);
	std::swap(contact.node1, contact.node2);
	std::swap(contact.surfaceHit1, contact.surfaceHit2);
	std::swap(contact.normal1, contact.normal2);
	return contact;
}
ContactData computeCollision(Node* nodePlane, Node* nodeSphere, const ColliderCube& cube1, const ColliderCube& cube2)
{
	AKA_NOT_IMPLEMENTED;
	return ContactData::Invalid;
}
ContactData computeCollision(Node* nodePlane, Node* nodeSphere, const ColliderPlane& plane1, const ColliderPlane& plane2)
{
	AKA_NOT_IMPLEMENTED;
	return ContactData::Invalid;
}
ContactData computeCollision(Node* nodePlane, Node* nodeSphere, const ColliderSphere& sphere, const ColliderCube& cube)
{
	AKA_NOT_IMPLEMENTED;
	return ContactData::Invalid;
}
ContactData computeCollision(Node* nodePlane, Node* nodeSphere, const ColliderCube& cube, const ColliderSphere& sphere)
{
	AKA_NOT_IMPLEMENTED;
	return ContactData::Invalid;
}
ContactData computeCollision(Node* nodePlane, Node* nodeSphere, const ColliderPlane& plane, const ColliderCube& cube)
{
	AKA_NOT_IMPLEMENTED;
	return ContactData::Invalid;
}
ContactData computeCollision(Node* nodePlane, Node* nodeSphere, const ColliderCube& cube, const ColliderPlane& plane)
{
	AKA_NOT_IMPLEMENTED;
	return ContactData::Invalid;
}
template <typename ColliderShape1, typename ColliderShape2>
ContactData computeCollisionT(Node* nodeShape1, Node* nodeShape2, const ColliderShape& shape1, const ColliderShape& shape2)
{
	static_assert(std::is_base_of<ColliderShape, ColliderShape1>::value);
	static_assert(std::is_base_of<ColliderShape, ColliderShape2>::value);
	AKA_ASSERT(shape1.getShapeType() == ColliderShape1().getShapeType(), "Invalid type");
	AKA_ASSERT(shape2.getShapeType() == ColliderShape2().getShapeType(), "Invalid type");
	return computeCollision(nodeShape1, nodeShape2, reinterpret_cast<const ColliderShape1&>(shape1), reinterpret_cast<const ColliderShape2&>(shape2));
}
ContactData ColliderComponent::computeContactData(ColliderComponent& _other)
{
	switch (m_shape->getShapeType())
	{
	case ColliderShapeType::Cube:
		switch (_other.m_shape->getShapeType())
		{
		case ColliderShapeType::Cube:   return computeCollisionT<ColliderCube, ColliderCube>  (getNode(), _other.getNode(), *m_shape, *_other.m_shape);
		case ColliderShapeType::Sphere: return computeCollisionT<ColliderCube, ColliderSphere>(getNode(), _other.getNode(), *m_shape, *_other.m_shape);
		case ColliderShapeType::Plane:  return computeCollisionT<ColliderCube, ColliderPlane> (getNode(), _other.getNode(), *m_shape, *_other.m_shape);
		default: AKA_NOT_IMPLEMENTED;   return ContactData::Invalid;
		}
		AKA_UNREACHABLE;
		return ContactData::Invalid;
	case ColliderShapeType::Sphere:
		switch (_other.m_shape->getShapeType())
		{
		case ColliderShapeType::Cube:   return computeCollisionT<ColliderSphere, ColliderCube>  (getNode(), _other.getNode(), *m_shape, *_other.m_shape);
		case ColliderShapeType::Sphere: return computeCollisionT<ColliderSphere, ColliderSphere>(getNode(), _other.getNode(), *m_shape, *_other.m_shape);
		case ColliderShapeType::Plane:  return computeCollisionT<ColliderSphere, ColliderPlane> (getNode(), _other.getNode(), *m_shape, *_other.m_shape);
		default: AKA_NOT_IMPLEMENTED;   return ContactData::Invalid;
		}
		AKA_UNREACHABLE;
		return ContactData::Invalid;
	case ColliderShapeType::Plane: {
		switch (_other.m_shape->getShapeType())
		{
		case ColliderShapeType::Cube:   return computeCollisionT<ColliderPlane, ColliderCube>  (getNode(), _other.getNode(), *m_shape, *_other.m_shape);
		case ColliderShapeType::Sphere: return computeCollisionT<ColliderPlane, ColliderSphere>(getNode(), _other.getNode(), *m_shape, *_other.m_shape);
		case ColliderShapeType::Plane:  return computeCollisionT<ColliderPlane, ColliderPlane> (getNode(), _other.getNode(), *m_shape, *_other.m_shape);
		default: AKA_NOT_IMPLEMENTED;   return ContactData::Invalid;
		}
		AKA_UNREACHABLE;
		return ContactData::Invalid;
	}
	default: 
		AKA_NOT_IMPLEMENTED; 
		return ContactData::Invalid;
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