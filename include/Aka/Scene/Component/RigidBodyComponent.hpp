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