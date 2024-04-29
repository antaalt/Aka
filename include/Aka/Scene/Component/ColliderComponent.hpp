#pragma once 

#include <Aka/Scene/Component.hpp>

namespace aka {

struct RigidBody;

struct ContactData
{
	// Rigid body 1 impacted in collision
	RigidBody* rb1;
	// Rigid body 2 impacted in collision
	RigidBody* rb2;
	// Contact point of the entity 1 in local space.
	vec3f surfaceHit1;
	// Contact point of the entity 2 in local space.
	vec3f surfaceHit2;
	// Contact normal of the entity 1 in local space
	vec3f normal1;
	// Contact normal of the entity 2 in local space
	vec3f normal2;
	// Penetration depth, aka magnitude of the collision.
	float penetration;
};

enum class ColliderShapeType {
	Sphere,
	Plane,
	Cube,
};
// http://iquilezles.org/articles/distfunctions/
struct ColliderShape
{
	ColliderShape(ColliderShapeType _shape) : m_shape(_shape) {}
	virtual ~ColliderShape() {}

	//virtual bool isColliding(const ColliderShape& lhs, ContactData& contact) = 0;
	// Compute distance from shape with given point.
	virtual float sdf(const vec3f& _from) = 0;
	ColliderShapeType getShapeType() const { return m_shape; }
private:
	ColliderShapeType m_shape;
};

// RigidBody
//	Hold informations about rigidity & interaction to gravity
// Collider
//	Hold informations about shape
// Physic system
//	A physic system will apply gravity to every rigid body.
//	Then it will gather collisions by checking every collider with a rigid body against every other ones (Ideally inside a BVH or octree).
//	Then it will solve collisions for every rigid body with colliders against every other colliders (rigid or not)
//		We then must not store everything within RigidBody. 
//		Or every collider own a rigid body.
//		And we have a kind of physicComponent that store both collider & rigidbody. But we could have rigid body without colliders...
//
/*ContactData computeCollisionSphereToSphere(const vec3f& position1, float radius1, const vec3f& position2, float radius2)
{
	vec3f direction = position2 - position1;
	float distance = vec3f::length(direction);
	ContactData contact;
	contact.rb1 = ; // These should not be here...
	contact.rb2 = ;
	contact.surfaceHit1 = position1 + direction * radius1;
	contact.surfaceHit2 = position2 - direction * radius2;
	contact.normal1 = vec3f::normalize(direction);
	contact.normal2 = -contact.normal1;
	contact.penetration = distance - radius1 - radius2;
	return contact;
}

ContactData computeCollisionSphereToPlane(const vec3f& spherePosition, float sphereRadius, const vec3f& pointOnPlane, const vec3f& planeNormal)
{
	vec3f direction = -planeNormal;
	float distance = vec3f::dot(planeNormal, spherePosition - pointOnPlane) - sphereRadius;
	ContactData contact;
	contact.rb1 = ; // These should not be here...
	contact.rb2 = ;
	contact.surfaceHit1 = spherePosition + direction * sphereRadius;
	contact.surfaceHit2 = spherePosition + direction * distance;
	contact.normal1 = vec3f::normalize(direction);
	contact.normal2 = planeNormal;
	contact.penetration = distance - sphereRadius;
	return contact;
}*/

struct ColliderSphere : ColliderShape
{
	ColliderSphere() : ColliderShape(ColliderShapeType::Sphere) {}
	float sdf(const vec3f& _from) override {
		float dist = _from.norm();
		return dist - m_radius;
	}
	ContactData getContact(const vec3f& _from)
	{
		// Assume collision here, with negative dist.
		float dist = sdf(_from);
	}
	float m_radius;
};
struct ColliderCube : ColliderShape
{

	ColliderCube() : ColliderShape(ColliderShapeType::Cube) {}
	float sdf(const vec3f& _from) override {
		vec3f q = vec3f::abs(_from) - extent;
		return vec3f::length(vec3f::max(q, vec3f(0.f))) + min(max(q.x, max(q.y, q.z)), 0.f);
	}
	vec3f extent;
};
struct ColliderPlane : ColliderShape
{
	ColliderPlane() : ColliderShape(ColliderShapeType::Plane) {}

	float sdf(const vec3f& _from) override {
		// normal must be normalized
		return vec3f::dot(_from, normal) + h;
	}
	float h;
	norm3f normal;
};

struct ArchiveColliderComponent : ArchiveComponent
{
	ArchiveColliderComponent(ArchiveComponentVersionType _version);
	
	ColliderShapeType shape;
	
	void parse(BinaryArchive& archive) override;
};
class ColliderComponent : public Component<ColliderComponent, ArchiveColliderComponent>
{
public:
	ColliderComponent(Node* node);
	~ColliderComponent();

	void onBecomeActive(AssetLibrary* library, Renderer* _renderer) override;
	void onBecomeInactive(AssetLibrary* library, Renderer* _renderer) override;
	void onRenderUpdate(AssetLibrary* library, Renderer* _renderer) override;
	void onUpdate(Time _time) override;

public:
	// TODO: compute collision result (impact, force & co...)
	float computeDistance(ColliderComponent* _other);
public:
	void fromArchive(const ArchiveColliderComponent& archive) override;
	void toArchive(ArchiveColliderComponent& archive) override;
private:
	ColliderShape* m_shape;
};

AKA_DECL_COMPONENT(ColliderComponent);
};