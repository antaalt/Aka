#pragma once 

#include <Aka/Scene/Component.hpp>

namespace aka {

struct ContactData
{
	// Node 1 impacted in collision
	Node* node1 = nullptr;
	// Node 2 impacted in collision
	Node* node2 = nullptr;
	// Contact point of the entity 1 in local space.
	point3f surfaceHit1 = point3f::zero();
	// Contact point of the entity 2 in local space.
	point3f surfaceHit2 = point3f::zero();
	// Contact normal of the entity 1 in local space
	vec3f normal1 = vec3f::up();
	// Contact normal of the entity 2 in local space
	vec3f normal2 = vec3f::up();
	// Penetration depth, aka magnitude of the collision.
	float penetration = 0.f;

	static const ContactData Invalid;

	bool isColliding() const;
};

enum class ColliderShapeType
{
	Unknown,

	Sphere,
	Plane,
	Cube,
};
struct ColliderShape
{
	ColliderShape(ColliderShapeType _shape) : m_shape(_shape) {}
	virtual ~ColliderShape() {}

	ColliderShapeType getShapeType() const { return m_shape; }
private:
	ColliderShapeType m_shape;
};

struct ColliderSphere : ColliderShape
{
	ColliderSphere() : ColliderShape(ColliderShapeType::Sphere) {}
	// TODO: get these data somehow
	point3f m_offset = point3f(0, 0, 0); // Collider offset from node transform
	float m_radius = 1.f;
};
struct ColliderCube : ColliderShape
{

	ColliderCube() : ColliderShape(ColliderShapeType::Cube) {}
	vec3f m_extent;
};
struct ColliderPlane : ColliderShape
{
	ColliderPlane() : ColliderShape(ColliderShapeType::Plane) {}
	point3f m_offset = point3f(0, 0, 0); // Collider offset from node transform
	vec3f m_normal = vec3f(0, 1, 0);
};

struct ArchiveColliderComponent : ArchiveComponent
{
	ArchiveColliderComponent(ArchiveComponentVersionType _version);
	
	ColliderShapeType shape = ColliderShapeType::Unknown;
	
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
	ContactData computeContactData(ColliderComponent& _other);
	ColliderShapeType getShapeType() const { return m_shape->getShapeType(); }
	const ColliderShape* getShape() const { return m_shape; }
public:
	void fromArchive(const ArchiveColliderComponent& archive) override;
	void toArchive(ArchiveColliderComponent& archive) override;
private:
	ColliderShape* m_shape;
};

AKA_DECL_COMPONENT(ColliderComponent);
};