#include <Aka/Scene/Component.hpp>

#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

uint32_t Component::s_globalComponentID = 0;

Component::Component(ComponentID id) :
	m_state(ComponentState::PendingActivation),
	m_id(id)
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