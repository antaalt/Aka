#include <Aka/Scene/Component/StaticMeshComponent.hpp>

namespace aka {

StaticMeshComponent::StaticMeshComponent(AssetID assetID) :
	Component(Component::generateID<StaticMeshComponent>()),
	m_assetID(assetID)
{
}
StaticMeshComponent::~StaticMeshComponent()
{
	// Do not own instance, neither does mesh.
	// Need to tell the instance to be deleted
}
void StaticMeshComponent::setInstance(Instance* instance)
{
	m_instance = instance;
}
void StaticMeshComponent::setMesh(ResourceHandle<StaticMesh> mesh)
{
	m_mesh= mesh;
}
ResourceHandle<StaticMesh> StaticMeshComponent::getMesh()
{
	return m_mesh;
}
};