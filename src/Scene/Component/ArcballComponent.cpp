#include <Aka/Scene/Component/ArcballComponent.hpp>

#include <Aka/Platform/Input.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Core/Application.h>

namespace aka {

ArchiveArcballComponent::ArchiveArcballComponent(ArchiveComponentVersionType _version) :
	ArchiveComponent(Component<ArcballComponent, ArchiveArcballComponent>::getComponentID(), _version)
{
}

ArcballComponent::ArcballComponent(Node* node) :
	Component(node),
	m_updateEnabled(true),
	m_dirtyBounds(false),
	m_bounds(),
	m_controller(nullptr)
{
}

ArcballComponent::~ArcballComponent()
{
}

void ArchiveArcballComponent::parse(BinaryArchive& archive)
{
}

void ArcballComponent::setBounds(const aabbox<>& bounds)
{
	m_bounds = bounds;
	m_dirtyBounds = true;
}
void ArcballComponent::setUpdateEnabled(bool value)
{
	m_updateEnabled = value;
}
bool ArcballComponent::isUpdateEnabled() const
{
	return m_updateEnabled;
}
void ArcballComponent::onBecomeActive(AssetLibrary* library, Renderer* _renderer)
{
	m_controller = mem::akaNew<CameraArcball>(AllocatorMemoryType::Object, AllocatorCategory::Global, _renderer->getWindow());
	m_controller->set(m_bounds);
	getNode()->setLocalTransform(m_controller->transform());
}
void ArcballComponent::onBecomeInactive(AssetLibrary* library, Renderer* _renderer)
{
	mem::akaSafeDelete(m_controller);
}
void ArcballComponent::onUpdate(Time deltaTime)
{
	if (m_dirtyBounds)
	{
		m_controller->set(m_bounds);
		m_dirtyBounds = false;
	}
	if (m_updateEnabled && m_controller->update(deltaTime))
	{
		getNode()->setLocalTransform(m_controller->transform());
	}
}
void ArcballComponent::fromArchive(const ArchiveArcballComponent& archive)
{
}

void ArcballComponent::toArchive(ArchiveArcballComponent& archive)
{
}

};
