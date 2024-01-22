#pragma once 

#include <Aka/Scene/Component.hpp>

namespace aka {

class ColliderComponent
{
	// bbox ? shape depend on content...
	// Check collisions with all other components...
	// Only between rigid body & colliders..
	void update(Time deltaTime);
};

// TODO we have an issue here which is component ordering.
// If we cant determine an order for components, we lose.
// Some components need to be updated before others...
// Should have some global component pool.
// We do not need to iterate on nodes recursively as well...
// might be interesting if we want to skip some nodes for examples, but other way its not for perf.
// Not a main issue though, firstly need a component update ordering.
struct ArchiveRigidBodyComponent : ArchiveComponent
{
	ArchiveRigidBodyComponent(ArchiveComponentVersionType _version);

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
	void onUpdate(Time _time) override;

	const vec3f& getVelocity() const { return m_velocity; }
public:
	void fromArchive(const ArchiveRigidBodyComponent& archive) override;
	void toArchive(ArchiveRigidBodyComponent& archive) override;
private:
	vec3f m_velocity;
};

AKA_DECL_COMPONENT(RigidBodyComponent);
};