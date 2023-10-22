#include <Aka/Scene/Component/ArcballComponent.hpp>

#include <Aka/Platform/Input.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Core/Application.h>

namespace aka {

ArchiveArcballComponent::ArchiveArcballComponent() :
	ArchiveComponent(generateComponentID<ArcballComponent>(), 0)
{
}

ArcballComponent::ArcballComponent(Node* node) :
	Component(node, generateComponentID<ArcballComponent>())
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
void ArcballComponent::fromArchive(const ArchiveComponent& archive)
{
	AKA_ASSERT(archive.getComponentID() == getComponentID(), "Invalid ID");
	const ArchiveArcballComponent& a = reinterpret_cast<const ArchiveArcballComponent&>(archive);
}

void ArcballComponent::toArchive(ArchiveComponent& archive)
{
	AKA_ASSERT(archive.getComponentID() == getComponentID(), "Invalid ID");
	ArchiveArcballComponent& a = reinterpret_cast<ArchiveArcballComponent&>(archive);
}

};
