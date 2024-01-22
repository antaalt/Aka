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
	Component(node)
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
	m_controller.set(bounds);
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
	getNode()->setLocalTransform(m_controller.transform());
}
void ArcballComponent::onUpdate(Time deltaTime)
{
	if (m_updateEnabled && m_controller.update(deltaTime))
	{
		getNode()->setLocalTransform(m_controller.transform());
	}
}
void ArcballComponent::fromArchive(const ArchiveArcballComponent& archive)
{
}

void ArcballComponent::toArchive(ArchiveArcballComponent& archive)
{
}

};
