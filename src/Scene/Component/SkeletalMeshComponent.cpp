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
}
void SkeletalMeshComponent::onBecomeInactive(AssetLibrary* library, Renderer* _renderer)
{
	m_meshHandle.reset();
	_renderer->destroySkeletalMeshInstance(m_instance);
	m_instance = InstanceHandle::Invalid;
}
void SkeletalMeshComponent::onRenderUpdate(AssetLibrary* library, Renderer* _renderer)
{
	_renderer->updateSkeletalMeshInstanceTransform(m_instance, getNode()->getWorldTransform());
}
ResourceHandle<SkeletalMesh> SkeletalMeshComponent::getMesh()
{
	return m_meshHandle;
}

void SkeletalMeshComponent::fromArchive(const ArchiveComponent& archive)
{
	AKA_ASSERT(archive.getComponentID() == getComponentID(), "Invalid ID");
	const ArchiveSkeletalMeshComponent& a = reinterpret_cast<const ArchiveSkeletalMeshComponent&>(archive);
	m_assetID = a.assetID;
}

void SkeletalMeshComponent::toArchive(ArchiveComponent& archive)
{
	AKA_ASSERT(archive.getComponentID() == getComponentID(), "Invalid ID");
	ArchiveSkeletalMeshComponent& a = reinterpret_cast<ArchiveSkeletalMeshComponent&>(archive);
	a.assetID = m_assetID;
}

};