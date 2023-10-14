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
	m_instance(InstanceHandle::Invalid)
{
}
StaticMeshComponent::~StaticMeshComponent()
{
	AKA_ASSERT(m_instance == InstanceHandle::Invalid, "Instance not deleted");
}
void StaticMeshComponent::onBecomeActive(AssetLibrary* library, Renderer* _renderer)
{
	m_meshHandle = library->load<StaticMesh>(m_assetID, _renderer);
	m_instance = _renderer->createStaticMeshInstance(m_assetID);
}
void StaticMeshComponent::onBecomeInactive(AssetLibrary* library, Renderer* _renderer)
{
	m_meshHandle.reset();
	_renderer->destroyStaticMeshInstance(m_instance);
	m_instance = InstanceHandle::Invalid;
}
void StaticMeshComponent::onTransformChanged()
{
	m_updateGPU = true;
}
void StaticMeshComponent::onHierarchyChanged()
{
	m_updateGPU = true;
}
void StaticMeshComponent::onRenderUpdate(AssetLibrary* library, Renderer* _renderer)
{
	if (m_updateGPU)
	{
		_renderer->updateStaticMeshInstanceTransform(m_instance, getNode()->getWorldTransform());
		m_updateGPU = false;
	}
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