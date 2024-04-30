#pragma once

#include <Aka/OS/Archive.h>
#include <Aka/Core/Config.h>
#include <Aka/Core/Crc.hpp>
#include <Aka/Core/Container/String.h>
#include <Aka/Core/Container/HashMap.hpp>
#include <Aka/Scene/ComponentType.hpp>
#include <Aka/Scene/ComponentAllocator.hpp>
#include <Aka/Graphic/GraphicDevice.h>

#include <iostream>

namespace aka {

class AssetLibrary;
class Renderer;
class Node;
class ComponentBase;
struct Archive;
struct ArchiveComponent;


using ComponentSet = TreeSet<ComponentID>;
using ComponentMap = TreeMap<ComponentID, ComponentBase*>;

struct ArchiveComponent
{
	ArchiveComponent(ComponentID componentID, ArchiveComponentVersionType version);

	void load(const Vector<byte_t>& byte);
	void save(Vector<byte_t>& byte);
protected:
	virtual void parse(BinaryArchive& archive) = 0;
public:
	ComponentID getComponentID() const { return m_id; }
	ArchiveComponentVersionType getVersion() const { return m_version; }
private:
	ComponentID m_id;
	ArchiveComponentVersionType m_version;
};

class ComponentBase
{
public:
	ComponentBase() = delete;
	ComponentBase(Node* node, ComponentID componentID);
	virtual ~ComponentBase() {}

protected:
	friend class Node;
	friend class Scene;

	void attach();
	void detach();
	void activate(AssetLibrary* library, Renderer* _renderer);
	void deactivate(AssetLibrary* library, Renderer* _renderer);

public:
	virtual void fromArchiveBase(const ArchiveComponent& archive) = 0;
	virtual void toArchiveBase(ArchiveComponent& archive) = 0;
	virtual ArchiveComponent* createArchiveBase(ArchiveComponentVersionType _version = 0) = 0;
	virtual void destroyArchiveBase(ArchiveComponent* _archive) = 0;
protected:
	void transformUpdate();
	void hierarchyUpdate();
	void update(Time deltaTime);
	void fixedUpdate(Time deltaTime);
	void renderUpdate(AssetLibrary* library, Renderer* _renderer);
protected:
	// When a component just got attached, called just after constructor
	virtual void onAttach() {}
	// When a component just got detached, called just before destructor
	virtual void onDetach() {}
	// When a component become active.
	virtual void onBecomeActive(AssetLibrary* library, Renderer* _renderer) {}
	// When a component become inactive.
	virtual void onBecomeInactive(AssetLibrary* library, Renderer* _renderer) {}

	// When the node transform was updated
	virtual void onTransformChanged() { unregisterForUpdates(ComponentUpdateFlags::TransformUpdate); }
	// When the node hierarchy (and transform) was updated
	virtual void onHierarchyChanged() { unregisterForUpdates(ComponentUpdateFlags::HierarchyUpdate); }

	// When we update the component
	virtual void onUpdate(Time deltaTime) { unregisterForUpdates(ComponentUpdateFlags::Update); }
	// When we update the component with fixed time step.
	virtual void onFixedUpdate(Time deltaTime) { unregisterForUpdates(ComponentUpdateFlags::FixedUpdate); }
	// When update to renderer are required.
	virtual void onRenderUpdate(AssetLibrary* library, Renderer* _renderer) { unregisterForUpdates(ComponentUpdateFlags::RenderUpdate); }
public:
	// Mark component as dirty
	void setDirty() { m_dirty = true; }
	// Check if component is dirty
	bool isDirty() const { return m_dirty; }
protected:
	// Clear component as dirty
	void clearDirty() { m_dirty = false; }
public:
	// Get the owner node of the component
	Node* getNode() { return m_node; }
	// Get the owner node of the component
	const Node* getNode() const { return m_node; }
	// Get component ID, using type for now for reliable constant value
	ComponentID getComponentID() const { return m_componentID; }
	// Get component state.
	ComponentState getState() const { return m_state; }
	// Get component state.
	ComponentUpdateFlags getUpdateFlags() const { return m_updateFlags; }
private:
	// Register for internal updates
	void registerForUpdates(ComponentUpdateFlags flags);
	// Unregister from internal updates
	void unregisterForUpdates(ComponentUpdateFlags flags);
	// Check if flags are registered
	bool areRegistered(ComponentUpdateFlags flags);
private:
	bool m_dirty;				// Has the component been updated somehow
	Node* m_node;				// Node the component is part of.
	ComponentID m_componentID;	// ID of the component
	ComponentState m_state;		// Current state of the component
	ComponentUpdateFlags m_updateFlags; // Update flags of component.
};

template <typename T, typename A>
struct Component : ComponentBase
{
	static_assert(std::is_base_of<ArchiveComponent, A>::value);
public:
	using Archive = A;
	Component(Node* _node);
	virtual ~Component() {};
	static const char* getName();
	static ComponentID getComponentID();
	virtual void fromArchive(const Archive& archive) = 0;
	virtual void toArchive(Archive& archive) = 0;
	Archive* createArchive();
	Archive* createArchive(ArchiveComponentVersionType _version);
	void destroyArchive(Archive* _archive);
protected:
	void fromArchiveBase(const ArchiveComponent& _archive) override;
	void toArchiveBase(ArchiveComponent& _archive);
	ArchiveComponent* createArchiveBase(ArchiveComponentVersionType _version = 0) override;
	void destroyArchiveBase(ArchiveComponent* _archive);
public:
	static ComponentAllocator<T>* create();
	static void destroy(ComponentAllocator<T>* _allocator);
private:
	static ComponentRegister<T> s_register;
};

template <typename T>
static constexpr const char* getComponentName() {
	static_assert(false); // This is being defined by AKA_DECL_COMPONENT
}

template <typename T, typename A>
Component<T, A>::Component(Node* _node) : 
	ComponentBase(_node, getComponentID()) 
{ 
	AKA_UNUSED(s_register); // Required to instantiate data
}
template <typename T, typename A>
const char* Component<T, A>::getName() {
	return getComponentName<T>();
}
template <typename T, typename A>
ComponentID Component<T, A>::getComponentID() {
	static const ComponentID id = static_cast<ComponentID>(WSID(getName()));
	return id;
}
template <typename T, typename A>
typename Component<T, A>::Archive* Component<T, A>::createArchive()
{
	return createArchive(0); // TODO: retrieve default version
}
template <typename T, typename A>
typename Component<T, A>::Archive* Component<T, A>::createArchive(ArchiveComponentVersionType _version)
{ 
	return mem::akaNew<Archive>(AllocatorMemoryType::Object, AllocatorCategory::Assets, _version);
}
template <typename T, typename A>
void Component<T, A>::destroyArchive(Archive* _archive)
{ 
	mem::akaDelete(_archive); 
}
template <typename T, typename A>
void Component<T, A>::fromArchiveBase(const ArchiveComponent& _archive) 
{
	AKA_ASSERT(_archive.getComponentID() == getComponentID(), "Invalid component archive");
	fromArchive(reinterpret_cast<const Archive&>(_archive));
}
template <typename T, typename A>
void Component<T, A>::toArchiveBase(ArchiveComponent& _archive) 
{
	AKA_ASSERT(_archive.getComponentID() == getComponentID(), "Invalid component archive");
	toArchive(reinterpret_cast<Archive&>(_archive));
}
template <typename T, typename A>
ArchiveComponent* Component<T, A>::createArchiveBase(ArchiveComponentVersionType _version)
{
	return createArchive(_version);
}
template <typename T, typename A>
void Component<T, A>::destroyArchiveBase(ArchiveComponent* _archive) 
{
	AKA_ASSERT(_archive->getComponentID() == getComponentID(), "Invalid component archive");
	destroyArchive(reinterpret_cast<Archive*>(_archive));
}
template<typename T, typename A>
ComponentAllocator<T>* Component<T, A>::create()
{
	return s_register.create();
}
template<typename T, typename A>
void Component<T, A>::destroy(ComponentAllocator<T>* _allocator)
{
	return s_register.destroy(_allocator);
}

template <typename T, typename A>
ComponentRegister<T> Component<T, A>::s_register = ComponentRegister<T>(Component<T, A>::getComponentID(), Component<T, A>::getName());

}; // namespace aka

#define AKA_DECL_COMPONENT(ComponentType)								\
template <>																\
inline static constexpr const char* getComponentName<ComponentType>() { \
	return AKA_STRINGIFY(ComponentType);								\
}
