#pragma once

#include <Aka/Scene/Component.hpp>
#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Resource/Resource/SkeletalMesh.hpp>
#include <Aka/Renderer/Instance.hpp>
#include <Aka/Resource/Archive/ArchiveScene.hpp>

namespace aka {

struct ArchiveSkeletalMeshComponent : ArchiveComponent
{
	ArchiveSkeletalMeshComponent();

	AssetID assetID;

	void parse(BinaryArchive& archive) override;
};

class SkeletalMeshComponent : public Component
{
public:
	SkeletalMeshComponent(Node* node);
	~SkeletalMeshComponent();

	void onBecomeActive(AssetLibrary* library, Renderer* _renderer) override;
	void onBecomeInactive(AssetLibrary* library, Renderer* _renderer) override;
	void onRenderUpdate(AssetLibrary* library, Renderer* _renderer) override;
	void onUpdate(Time deltaTime) override;

	ResourceHandle<SkeletalMesh> getMesh();
	const Vector<SkeletalMeshAnimation>& getAnimations() const;
	const Vector<SkeletalMeshBone>& getBones() const;
	const SkeletalMeshAnimation& getCurrentAnimation() const;
	uint32_t getCurrentAnimationIndex() const;
	void setCurrentAnimation(uint32_t index);
public:
	void fromArchive(const ArchiveComponent& archive) override;
	void toArchive(ArchiveComponent& archive) override;
private:
	AssetID m_assetID;
	uint32_t m_currentAnimation;
	uint32_t m_rootBoneIndex;
	Vector<SkeletalMeshAnimation> m_animations;
	Vector<SkeletalMeshBone> m_bones; // instantiated bones.
	ResourceHandle<SkeletalMesh> m_meshHandle;
	InstanceHandle m_instance;
};

AKA_DECL_COMPONENT(SkeletalMeshComponent);

};