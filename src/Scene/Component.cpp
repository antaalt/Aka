#include <Aka/Scene/Component.hpp>

#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

std::map<ComponentID, ComponentAllocator*> ComponentAllocator::m_allocators;

void ComponentAllocator::unregisterAllocator(ComponentID id)
{
	AKA_ASSERT(m_allocators.count(id), "Component type not registered.");
	delete m_allocators[id];
	m_allocators.erase(id);
}

Component* ComponentAllocator::allocate(ComponentID id)
{
	AKA_ASSERT(m_allocators.count(id), "Component type not registered.");
	return m_allocators[id]->allocate_internal();
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
	m_allocators[component->id]->freeArchive_internal(component);
}

Component::Component(ComponentID componentID) :
	m_state(ComponentState::PendingActivation),
	m_componentID(componentID)
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
void Component::update(Time deltaTime)
{
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onUpdate(deltaTime);
}
void Component::fixedUpdate(Time deltaTime)
{
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onFixedUpdate(deltaTime);
}
void Component::renderUpdate(AssetLibrary* library, Renderer* _renderer)
{
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onRenderUpdate(library, _renderer);
}


};