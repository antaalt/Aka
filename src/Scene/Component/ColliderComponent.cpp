#include <Aka/Scene/Component/ColliderComponent.hpp>
#include <Aka/Renderer/Renderer.hpp>
#include <Aka/OS/Stream/MemoryStream.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveColliderComponent::ArchiveColliderComponent(ArchiveComponentVersionType _version) :
	ArchiveComponent(Component<ColliderComponent, ArchiveColliderComponent>::getComponentID(), _version)
{
}

void ArchiveColliderComponent::parse(BinaryArchive& archive)
{
	archive.parse<ShapeType>(shape);
}

ColliderComponent::ColliderComponent(Node* node) :
	Component(node),
	m_shape(nullptr)
{
}
ColliderComponent::~ColliderComponent()
{
	delete m_shape;
}
void ColliderComponent::onBecomeActive(AssetLibrary* library, Renderer* _renderer)
{
}
void ColliderComponent::onBecomeInactive(AssetLibrary* library, Renderer* _renderer)
{
}
void ColliderComponent::onRenderUpdate(AssetLibrary* library, Renderer* _renderer)
{
}
void ColliderComponent::onUpdate(Time deltaTime)
{
}
float ColliderComponent::computeDistance(ColliderComponent* _other)
{
	point3f currentPos = getNode()->getWorldTransform().multiplyPoint(point3f(0.f));
	point3f otherPos = _other->getNode()->getWorldTransform().multiplyPoint(point3f(0.f));
	float distanceBetweenBoth = point3f::distance(currentPos, otherPos);
	float distanceFromOther = m_shape->sdf(otherPos - currentPos);
	float distanceFromCurrent = _other->m_shape->sdf(currentPos - otherPos);
	return distanceBetweenBoth - (distanceBetweenBoth - distanceFromOther) - (distanceBetweenBoth - distanceFromCurrent);
}
void ColliderComponent::fromArchive(const ArchiveColliderComponent& archive)
{
	AKA_ASSERT(m_shape == nullptr, "");
	switch (archive.shape) {
	case ShapeType::Box:
		m_shape = new Box3D;
		break;
	case ShapeType::Sphere:
		m_shape = new Sphere3D;
		break;
	case ShapeType::Plane:
		m_shape = new Plane3D;
		break;
	default:
		AKA_NOT_IMPLEMENTED;
		break;
	}
}
void ColliderComponent::toArchive(ArchiveColliderComponent& archive)
{
	archive.shape = m_shape->getShapeType();
}

};