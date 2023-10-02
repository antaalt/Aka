#include <Aka/Scene/Component/StaticMeshComponent.hpp>
#include <Aka/Renderer/Renderer.hpp>
#include <Aka/OS/Stream/MemoryStream.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveStaticMeshComponent::ArchiveStaticMeshComponent() :
	ArchiveComponent(generateComponentID<StaticMeshComponent>(), 0),
	assetID(AssetID::Invalid)
{
}

void ArchiveStaticMeshComponent::parse(BinaryArchive& archive)
{
	archive.parse<AssetID>(assetID);
}

StaticMeshComponent::StaticMeshComponent(Node* node) :
	Component(node, generateComponentID<StaticMeshComponent>()),
	m_assetID(AssetID::Invalid),
	m_instance(nullptr)
{
}
StaticMeshComponent::~StaticMeshComponent()
{
	// Do not own instance, neither does mesh.
	// Need to tell the instance to be deleted
	AKA_ASSERT(m_instance == nullptr, "Instance not deleted");
}
void StaticMeshComponent::onBecomeActive(AssetLibrary* library, Renderer* _renderer)
{
	m_meshHandle = library->load<StaticMesh>(m_assetID, _renderer);
	m_instance = _renderer->createInstance(m_assetID);
}
void StaticMeshComponent::onBecomeInactive(AssetLibrary* library, Renderer* _renderer)
{
	m_meshHandle.reset();
	_renderer->destroyInstance(m_instance);
}
void StaticMeshComponent::onRenderUpdate(AssetLibrary* library, Renderer* _renderer)
{
	m_instance->transform = getNode()->getWorldTransform();
}
ResourceHandle<StaticMesh> StaticMeshComponent::getMesh()
{
	return m_meshHandle;
}

void StaticMeshComponent::fromArchive(const ArchiveComponent& archive)
{
	AKA_ASSERT(archive.getComponentID() == getComponentID(), "Invalid ID");
	const ArchiveStaticMeshComponent& a = reinterpret_cast<const ArchiveStaticMeshComponent&>(archive);
	m_assetID = a.assetID;
}

void StaticMeshComponent::toArchive(ArchiveComponent& archive)
{
	AKA_ASSERT(archive.getComponentID() == getComponentID(), "Invalid ID");
	ArchiveStaticMeshComponent& a = reinterpret_cast<ArchiveStaticMeshComponent&>(archive);
	a.assetID = m_assetID;
}

};