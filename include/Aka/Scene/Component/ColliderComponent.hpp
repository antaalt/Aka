#pragma once 

#include <Aka/Scene/Component.hpp>

namespace aka {


enum class ShapeType {
	Sphere,
	Box,
	Plane,
};
// http://iquilezles.org/articles/distfunctions/
struct Shape3D {
	Shape3D(ShapeType _shape) : m_shape(_shape) {}
	// Compute distance from shape with given point.
	virtual float sdf(const vec3f& _from) = 0;
	ShapeType getShapeType() const { return m_shape; }
private:
	ShapeType m_shape;
};

struct Sphere3D : Shape3D {

	Sphere3D() : Shape3D(ShapeType::Sphere) {}
	float sdf(const vec3f& _from) override {
		float dist = _from.norm();
		return dist - m_radius;
	}
	float m_radius;
};
struct Box3D : Shape3D {

	Box3D() : Shape3D(ShapeType::Box) {}
	float sdf(const vec3f& _from) override {
		vec3f q = vec3f::abs(_from) - extent;
		return vec3f::length(vec3f::max(q, vec3f(0.f))) + min(max(q.x, max(q.y, q.z)), 0.f);
	}
	vec3f extent;
};
struct Plane3D : Shape3D {
	Plane3D() : Shape3D(ShapeType::Plane) {}

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
	
	ShapeType shape;
	
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
	Shape3D* m_shape;
};

AKA_DECL_COMPONENT(ColliderComponent);
};