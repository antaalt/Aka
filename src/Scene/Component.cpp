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

ComponentBase::ComponentBase(Node* node, ComponentID componentID) :
	m_state(ComponentState::PendingActivation),
	m_componentID(componentID),
	m_node(node),
	m_updateFlags(ComponentUpdateFlags::All)
{
}

void ComponentBase::attach()
{
	AKA_ASSERT(m_state == ComponentState::PendingActivation, "Invalid state");
	onAttach();
}
void ComponentBase::detach()
{
	onDetach();
}
void ComponentBase::activate(AssetLibrary* library, Renderer* _renderer)
{
	AKA_ASSERT(m_state == ComponentState::PendingActivation, "Invalid state");
	onBecomeActive(library, _renderer);
	m_state = ComponentState::Active;
}
void ComponentBase::deactivate(AssetLibrary* library, Renderer* _renderer)
{
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onBecomeInactive(library, _renderer);
	m_state = ComponentState::PendingDestruction;
}
void ComponentBase::transformUpdate()
{
	if (!areRegistered(ComponentUpdateFlags::TransformUpdate))
		return;
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onTransformChanged();
}
void ComponentBase::hierarchyUpdate()
{
	if (!areRegistered(ComponentUpdateFlags::HierarchyUpdate))
		return;
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onHierarchyChanged();
}
void ComponentBase::update(Time deltaTime)
{
	if (!areRegistered(ComponentUpdateFlags::Update))
		return;
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onUpdate(deltaTime);
}
void ComponentBase::fixedUpdate(Time deltaTime)
{
	if (!areRegistered(ComponentUpdateFlags::FixedUpdate))
		return;
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onFixedUpdate(deltaTime);
}
void ComponentBase::renderUpdate(AssetLibrary* library, Renderer* _renderer)
{
	if (!areRegistered(ComponentUpdateFlags::RenderUpdate))
		return;
	AKA_ASSERT(m_state == ComponentState::Active, "Invalid state");
	onRenderUpdate(library, _renderer);
}
void ComponentBase::registerForUpdates(ComponentUpdateFlags flags)
{
	m_updateFlags |= flags;
}
void ComponentBase::unregisterForUpdates(ComponentUpdateFlags flags)
{
	m_updateFlags &= ~flags;
}
bool ComponentBase::areRegistered(ComponentUpdateFlags flags)
{
	return asBool(m_updateFlags & flags);
}

FactoryBase::FactoryBase(ComponentID _component) 
{
	getFactoryMap().insert(std::make_pair(_component, this)); 
}

ComponentBase* FactoryBase::make(ComponentID _id, Node* _owner) {
	return getFactoryMap()[_id]->allocate(_owner);
}
void FactoryBase::unmake(ComponentBase* _component) {
	getFactoryMap()[_component->getComponentID()]->free(_component);
}
std::map<ComponentID, FactoryBase*>& FactoryBase::getFactoryMap() {
	static std::map<ComponentID, FactoryBase*> s_factories;
	return s_factories;
}

};