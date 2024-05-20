#pragma once 

#include <Aka/Scene/Component.hpp>
#include <Aka/Scene/Component/ColliderComponent.hpp>

namespace aka {

struct ArchiveRigidBodyComponent : ArchiveComponent
{
	ArchiveRigidBodyComponent(ArchiveComponentVersionType _version);

	float mass = 0.f;

	void parse(BinaryArchive& archive) override;
};
class RigidBodyComponent : public Component<RigidBodyComponent, ArchiveRigidBodyComponent>
{
public:
	RigidBodyComponent(Node* node);
	~RigidBodyComponent();

	void onBecomeActive(AssetLibrary* library, Renderer* _renderer) override;
	void onBecomeInactive(AssetLibrary* library, Renderer* _renderer) override;
	void onRenderUpdate(AssetLibrary* library, Renderer* _renderer) override;
	void onFixedUpdate(Time _time) override;
	void onUpdate(Time _time) override;
public:
	void fromArchive(const ArchiveRigidBodyComponent& archive) override;
	void toArchive(ArchiveRigidBodyComponent& archive) override;
public:
	// Should use world transform instead ?
	//const point3f& getPosition() const { return m_position; }
	//const quatf& getOrientation() const { return m_orientation; }
	//const point3f& getPreviousPosition() const { return m_previousPosition; }
	//const quatf& getPreviousOrientation() const { return m_previousOrientation; }

	float getMass() const { return 1.f / m_massInverse; }
	void setMass(float _mass) { m_massInverse = 1.f / _mass; }
private:
	// Get the XPBD generalized inverse mass
	float generalizedInverseMass(const vec3f& hitPoint, const vec3f& correction) const;
	// Check if it is a dynamic rigid body
	bool isDynamic() const;
	// Solve position constraint between 2 rigid bodies.
	static void solvePositionConstraint(const ContactData& contact, float dt);
private:
	point3f m_position = point3f::zero(); // m
	quatf m_orientation = quatf::identity(); // rad
	vec3f m_velocity = vec3f::zero(); // m/s
	vec3f m_angularVelocity = vec3f::zero(); // rad/s
	// TODO: computeSphereInertiaMatrix
	mat3f m_inertiaInverse = mat3f::identity(); // represent the center of gravity of object, based on center of mass.
	float m_massInverse = 0.f; // 1/kg2, mass inverse of zero means static, unmovable object (with infinite mass)

	// Cache
	point3f m_previousPosition = point3f::zero();
	quatf m_previousOrientation = quatf::identity();
	float m_lagrange = 0.f;
};

AKA_DECL_COMPONENT(RigidBodyComponent);
};