#include <Aka/Scene/Component/SkeletalMeshComponent.hpp>
#include <Aka/Renderer/Renderer.hpp>
#include <Aka/OS/Stream/MemoryStream.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveSkeletalMeshComponent::ArchiveSkeletalMeshComponent() :
	ArchiveComponent(generateComponentID<SkeletalMeshComponent>(), 0),
	assetID(AssetID::Invalid)
{
}

void ArchiveSkeletalMeshComponent::parse(BinaryArchive& archive)
{
	archive.parse<AssetID>(assetID);
}

SkeletalMeshComponent::SkeletalMeshComponent(Node* node) :
	Component(node, generateComponentID<SkeletalMeshComponent>()),
	m_assetID(AssetID::Invalid),
	m_currentAnimation(0),
	m_instance(InstanceHandle::Invalid)
{
}
SkeletalMeshComponent::~SkeletalMeshComponent()
{
	AKA_ASSERT(m_instance == InstanceHandle::Invalid, "Instance not deleted");
}
void SkeletalMeshComponent::onBecomeActive(AssetLibrary* library, Renderer* _renderer)
{
	m_meshHandle = library->load<SkeletalMesh>(m_assetID, _renderer);
	m_instance = _renderer->createSkeletalMeshInstance(m_assetID);
	m_animations = m_meshHandle.get().getAnimations();
	m_bones = m_meshHandle.get().getBones();
	m_rootBoneIndex = m_meshHandle.get().getRootBoneIndex();
}
void SkeletalMeshComponent::onBecomeInactive(AssetLibrary* library, Renderer* _renderer)
{
	m_meshHandle.reset();
	_renderer->destroySkeletalMeshInstance(m_instance);
	m_instance = InstanceHandle::Invalid;
}
mat4f getLocalMatrix(const Vector<SkeletalMeshBoneAnimation>& anim, const Vector<SkeletalMeshBone>& bones, uint32_t current)
{
	if (current == SkeletalVertex::InvalidBoneIndex)
		return mat4f::identity();
	return getLocalMatrix(anim, bones, bones[current].parentIndex) * anim[current].localTransform;
}

void SkeletalMeshComponent::onRenderUpdate(AssetLibrary* library, Renderer* _renderer)
{
	if (getNode()->has(NodeUpdateFlag::TransformUpdated))
	{
		_renderer->updateSkeletalMeshInstanceTransform(m_instance, getNode()->getWorldTransform());
	}

	// Should have a single texture storing all this. but frame dependent.
	const SkeletalMeshAnimation& animation = m_animations[m_currentAnimation];

	for (uint32_t iBone = 0; iBone < (uint32_t)animation.bones.size(); iBone++)
	{
		mat4f localTransform = getLocalMatrix(animation.bones, m_bones, iBone);
		mat4f boneMatrix = localTransform * m_bones[iBone].offset;
		_renderer->updateSkeletalMeshBoneInstance(m_instance, iBone, boneMatrix);
	}
}
void SkeletalMeshComponent::onUpdate(Time deltaTime)
{
	static float currentTime = 0.f;
	currentTime += deltaTime.seconds();
	// Select current animation

	// TODO could precompute all offset matrix in texture
	// https://developer.nvidia.com/gpugems/gpugems3/part-i-geometry/chapter-2-animated-crowd-rendering
	// GPU then need only :
	// - time
	// - animation data
	SkeletalMeshAnimation& animation = m_animations[m_currentAnimation];
	for (SkeletalMeshBoneAnimation& bone : animation.bones)
	{
		// TODO ticks to avoid animating when its useless.
		// + choose which space t o compute anim, tick space or second space
		float time = fmodf(currentTime * animation.tickPerSecond, animation.durationInTick);
		bone.update(time);
	}
}

};