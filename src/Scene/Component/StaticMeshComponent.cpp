#include <Aka/Scene/Component/StaticMeshComponent.hpp>
#include <Aka/Renderer/Renderer.hpp>

namespace aka {

StaticMeshComponent::StaticMeshComponent(AssetID assetID) :
	Component(Component::generateID<StaticMeshComponent>()),
	m_assetID(assetID),
	m_instance(nullptr)
{
}
StaticMeshComponent::~StaticMeshComponent()
{
	// Do not own instance, neither does mesh.
	// Need to tell the instance to be deleted
}
void StaticMeshComponent::onBecomeActive(AssetLibrary* library, Renderer* _renderer)
{
	m_meshHandle = library->get<StaticMesh>(m_assetID);
	m_instance = _renderer->createInstance(m_assetID);
}
void StaticMeshComponent::onBecomeInactive(AssetLibrary* library, Renderer* _renderer)
{
	m_meshHandle.reset();
	_renderer->destroyInstance(m_instance);
}
ResourceHandle<StaticMesh> StaticMeshComponent::getMesh()
{
	return m_meshHandle;
}
};