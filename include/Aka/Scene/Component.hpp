#pragma once

#include <Aka/OS/Archive.h>
#include <Aka/Core/Config.h>
#include <Aka/Core/Container/String.h>
#include <Aka/Graphic/GraphicDevice.h>

#include <set>
#include <map>

namespace aka {

class AssetLibrary;
class Renderer;
class Node;
class Component;
struct Archive;
struct ArchiveComponent;

enum class ComponentID : size_t { Invalid = (size_t)-1 };

enum class ComponentState
{
	PendingActivation,
	Active,
	PendingDestruction,
};

template <typename T> ComponentID generateComponentID();

struct ComponentAllocator
{
public:
	template <typename T>
	static void registerAllocator(ComponentID id);
	static void unregisterAllocator(ComponentID id);
	static Component* allocate(Node* node, ComponentID id);
	static void free(Component* component);
	static ArchiveComponent* allocateArchive(ComponentID id);
	static void freeArchive(ArchiveComponent* id);
private:
	static std::map<ComponentID, ComponentAllocator*> m_allocators;
protected:
	virtual Component* allocate_internal(Node* node) = 0;
	virtual void free_internal(Component* component) = 0;
	virtual ArchiveComponent* allocateArchive_internal() = 0;
	virtual void freeArchive_internal(ArchiveComponent* component) = 0;
};

using ComponentSet = std::set<ComponentID>;
using ComponentMap = std::map<ComponentID, Component*>;

using ArchiveComponentVersionType = uint32_t;

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


class Component
{
public:
	Component() = delete;
	Component(Node* node, ComponentID componentID);
	virtual ~Component() {}

protected:
	friend class Node;

	void attach();
	void detach();
	void activate(AssetLibrary* library, Renderer* _renderer);
	void deactivate(AssetLibrary* library, Renderer* _renderer);

public:
	virtual void fromArchive(const ArchiveComponent& archive) = 0;
	virtual void toArchive(ArchiveComponent& archive) = 0;
protected:
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

	// When we update the component
	virtual void onUpdate(Time deltaTime) {}
	// When we update the component with fixed time step.
	virtual void onFixedUpdate(Time deltaTime) {}
	// When update to renderer are required.
	// Should not be called every frame ?
	virtual void onRenderUpdate(AssetLibrary* library, Renderer* _renderer) {}
public:
	// Get the owner node of the component
	Node* getNode() { return m_node; }
	// Get the owner node of the component
	const Node* getNode() const { return m_node; }
	// Get component ID, using type for now for reliable constant value
	ComponentID getComponentID() const { return m_componentID; }
	// Get component state.
	ComponentState getState() const { return m_state; }
private:
	Node* m_node;				// Node the component is part of.
	ComponentID m_componentID;	// ID of the component
	ComponentState m_state;		// Current state of the component
};


template <typename T>
static void ComponentAllocator::registerAllocator(ComponentID id)
{
	static_assert(std::is_base_of<ComponentAllocator, T>::value);
	AKA_ASSERT(m_allocators.find(id) == m_allocators.end(), "Adding an allocator but one already exist");
	m_allocators.insert(std::make_pair(id, new T)); // Should use pool instead.
}

}; // namespace aka


// TODO should use compile time hash such as crc instead.
// This should be called within aka namespace. All components need to be in this namespace
#define AKA_DECL_COMPONENT(ComponentType) 															\
struct ComponentType ## Allocator : ComponentAllocator {											\
	Component* allocate_internal(Node* node) override { return new ComponentType(node); }			\
	void free_internal(Component* component) override { delete component; }							\
	ArchiveComponent* allocateArchive_internal() override { return new Archive ## ComponentType; }	\
	void freeArchive_internal(ArchiveComponent* component) override { delete component; }			\
};																									\
template <>																							\
inline ComponentID generateComponentID<ComponentType>() {											\
	static_assert(std::is_base_of<Component, ComponentType>::value);								\
	const char* name = AKA_STRINGIFY(ComponentType);												\
	std::size_t length = String::length(name);														\
	ComponentID id = static_cast<ComponentID>(hash::fnv(name, length));								\
	return id;																						\
}

#define AKA_REGISTER_COMPONENT(ComponentType) ComponentAllocator::registerAllocator<ComponentType ## Allocator>(generateComponentID<ComponentType>());
#define AKA_UNREGISTER_COMPONENT(ComponentType) ComponentAllocator::unregisterAllocator(generateComponentID<ComponentType>());

