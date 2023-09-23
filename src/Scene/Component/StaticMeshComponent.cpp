#include <Aka/Scene/Component/StaticMeshComponent.hpp>

namespace aka {

StaticMeshComponent::StaticMeshComponent() :
	Component(ComponentType::StaticMeshComponent)
{

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