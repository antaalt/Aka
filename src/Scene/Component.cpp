#include <Aka/Scene/Component.hpp>

#include <Aka/Resource/AssetLibrary.hpp>
#include <Aka/OS/Stream/MemoryStream.h>

namespace aka {

ArchiveComponent::ArchiveComponent(ComponentID id, ArchiveComponentVersionType version) :
	m_id(id),
	m_version(version)
{
}

void ArchiveComponent::load(const Vector<byte_t>& byte)
{
	MemoryReaderStream stream(byte);
	BinaryArchiveReader archive(stream);
	parse(archive);
	AKA_ASSERT(stream.offset() == stream.size(), "Failed to read whole data");
}

void ArchiveComponent::save(Vector<byte_t>& byte)
{
	MemoryWriterStream stream(byte);
	BinaryArchiveWriter archive(stream);
	parse(archive);
}

std::map<ComponentID, ComponentAllocator*> ComponentAllocator::m_allocators;

void ComponentAllocator::unregisterAllocator(ComponentID id)
{
	AKA_ASSERT(m_allocators.count(id), "Component type not registered.");
	delete m_allocators[id];
	m_allocators.erase(id);
}

Component* ComponentAllocator::allocate(Node* node, ComponentID id)
{
	AKA_ASSERT(m_allocators.count(id), "Component type not registered.");
	return m_allocators[id]->allocate_internal(node);
}
void ComponentAllocator::free(Component* component)
{
	m_allocators[component->getComponentID()]->free_internal(component);
}
ArchiveComponent* ComponentAllocator::allocateArchive(ComponentID id)
{
	AKA_ASSERT(m_allocators.count(id), "Component type not registered.");
	return m_allocators[id]->allocateArchive_internal();
}
void ComponentAllocator::freeArchive(ArchiveComponent* component)
{
	m_allocators[component->getComponentID()]->freeArchive_internal(component);
}

Component::Component(Node* node, ComponentID componentID) :
	m_state(ComponentState::PendingActivation),
	m_componentID(componentID),
	m_node(node),
	m_updateFlags(ComponentUpdateFlags::All)
{
}

void Component::attach()
{
	AKA_ASSERT(m_state == ComponentState::PendingActivation, "Invalid state");
	onAttach();
}
void Component::detach()
{
	onDetach();
}
void Component::activate(AssetLibrary* library, Renderer* _renderer)
{
	AKA_ASSERT(m_state == ComponentState::PendingActivation, "Invalid state");
	onBecomeActive(library, _renderer);
	m_state = ComponentState::Active;
}
void Component::deactivate(AssetLibrary* library, Renderer* _renderer)
{
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onBecomeInactive(library, _renderer);
	m_state = ComponentState::PendingDestruction;
}
void Component::transformUpdate()
{
	if (!areRegistered(ComponentUpdateFlags::TransformUpdate))
		return;
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onTransformChanged();
}
void Component::hierarchyUpdate()
{
	if (!areRegistered(ComponentUpdateFlags::HierarchyUpdate))
		return;
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onHierarchyChanged();
}
void Component::update(Time deltaTime)
{
	if (!areRegistered(ComponentUpdateFlags::Update))
		return;
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onUpdate(deltaTime);
}
void Component::fixedUpdate(Time deltaTime)
{
	if (!areRegistered(ComponentUpdateFlags::FixedUpdate))
		return;
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onFixedUpdate(deltaTime);
}
void Component::renderUpdate(AssetLibrary* library, Renderer* _renderer)
{
	if (!areRegistered(ComponentUpdateFlags::RenderUpdate))
		return;
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onRenderUpdate(library, _renderer);
}
void Component::registerForUpdates(ComponentUpdateFlags flags)
{
	m_updateFlags |= flags;
}
void Component::unregisterForUpdates(ComponentUpdateFlags flags)
{
	m_updateFlags &= ~flags;
}
bool Component::areRegistered(ComponentUpdateFlags flags)
{
	return asBool(m_updateFlags & flags);
}

};