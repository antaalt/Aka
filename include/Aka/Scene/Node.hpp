#pragma once

#include <Aka/Core/Container/String.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/Memory/Pool.h>

#include <Aka/Scene/Component.hpp>

namespace aka {

enum class NodeUpdateFlag : uint64_t
{
	None				= 0,

	Transform			= 1 << 0,
};
AKA_IMPLEMENT_BITMASK_OPERATOR(NodeUpdateFlag);

struct ComponentRange 
{
	ComponentRange(ComponentMap& iterator);

	ComponentMap::iterator begin() { return m_iterator; }
	ComponentMap::iterator end();
private:
	ComponentMap::iterator m_iterator;
};

class Node
{
public:
	Node();
	Node(const char* name);
	virtual ~Node();

	void attach(Component* component);
	// Attach a component to the entity
	template <typename T> T& attach();
	// Detach a component from the entity.
	template <typename T> void detach();
	// Get a component from the node
	template <typename T> T& get();
	// Check a component from the node
	template <typename T> bool has();

public:
	// Create the node data & all its components.
	void create(AssetLibrary* library, Renderer* renderer);
	// Destroy the node data & all its components.
	void destroy(AssetLibrary* library, Renderer* renderer);
	// Update the node data, and ensure components are up to date.
	void update(AssetLibrary* library, Renderer* renderer);
	// Update the component with delta time.
	void update(Time deltaTime);
	// Update the component with a fixed timestep.
	void fixedUpdate(Time deltaTime);
public:
	// Get node name
	const String& getName() const { return m_name; }
	// Does the node has active components
	bool isOrphan() const { return m_componentsActive.size() == 0; }
	// Get the active components
	const ComponentMap& getComponentMap() const { return m_componentsActive; }
	// Mark a component as dirty
	template<typename T> void setDirty();
	// Set update flag
	void setFlag(NodeUpdateFlag flag) { m_updateFlags |= flag; }
public:
	// Remove the node from the free, set its childs to its parent
	void unlink();
	// Add a node to this node.
	void addChild(Node* node);
	// Remove a node from this node.
	void removeChild(Node* node);
	// Set node parent
	void setParent(Node* parent);
	// Get node parent
	Node* getParent();
	// Get node parent
	const Node* getParent() const;
	// Get child count
	uint32_t getChildCount() const;
	// Get child n
	Node* getChild(uint32_t iChild);
	// Get child n
	const Node* getChild(uint32_t iChild) const;
private: // Hierarchy
	Node* m_parent;
	Vector<Node*> m_childrens;

public: // Transforms
	// Set local transform
	void setLocalTransform(const mat4f& transform);
	// Get the local transform
	mat4f& getLocalTransform();
	// Get the local transform
	const mat4f& getLocalTransform() const;
	// Get the world transform
	mat4f getWorldTransform() const;
	// Get the parent transform
	mat4f getParentTransform() const;
private:
	mat4f m_localTransform;
private: // Data
	String m_name;
	ComponentSet m_dirtyComponent;
	ComponentSet m_componentIDs;
	ComponentMap m_componentsActive;
	ComponentMap m_componentsToActivate;
	ComponentMap m_componentsToDeactivate;
private:
	NodeUpdateFlag m_updateFlags;
};

template<typename T>
inline T& Node::attach()
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	ComponentID id = generateComponentID<T>();
	AKA_ASSERT(!has<T>(), "Trying to attach already attached component");
	m_componentIDs.insert(id);
	T* component = reinterpret_cast<T*>(ComponentAllocator::allocate(this, id));
	component->onAttach();
	m_componentsToActivate.insert(std::make_pair(id, component));
	return *component;
}

template<typename T>
inline void Node::detach()
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	const ComponentID componentID = generateComponentID<T>();
	AKA_ASSERT(has<T>(), "Trying to detach non attached component");
	auto itActive = m_componentsActive.find(componentID);
	if (itActive != m_componentsActive.end())
	{
		// Component still active until deactivate was call on it.
		m_componentsToDeactivate.insert(std::make_pair(componentID, itActive->second));
		m_componentsActive.erase(componentID);
		return;
	}
	auto itToActivate = m_componentsToActivate.find(componentID);
	if (itToActivate != m_componentsToActivate.end())
	{
		// Component still active until deactivate was call on it.
		m_componentsToDeactivate.insert(std::make_pair(componentID, itToActivate->second));
		m_componentsToActivate.erase(itToActivate);
		return;
	}
	auto itToDeactivate = m_componentsToDeactivate.find(componentID);
	if (itToDeactivate != m_componentsToDeactivate.end())
	{
		AKA_ASSERT(false, "Trying to detach already detached component");
	}
	else
	{
		AKA_ASSERT(false, "Trying to detach non attached component");
		AKA_CRASH();
	}
}

template<typename T>
inline T& Node::get()
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	const ComponentID componentID = generateComponentID<T>();
	AKA_ASSERT(has<T>(), "Trying to get non attached component");
	auto itActive = m_componentsActive.find(componentID);
	if (itActive != m_componentsActive.end())
	{
		return *reinterpret_cast<T*>(itActive->second);
	}
	auto itToActivate = m_componentsToActivate.find(componentID);
	if (itToActivate != m_componentsToActivate.end())
	{
		return *reinterpret_cast<T*>(itToActivate->second);
	}
	auto itToDeactivate = m_componentsToDeactivate.find(componentID);
	if (itToDeactivate != m_componentsToDeactivate.end())
	{
		return *reinterpret_cast<T*>(itToDeactivate->second);
	}
	AKA_ASSERT(false, "Trying to get unattached component");
	AKA_CRASH();
}
template<typename T>
inline bool aka::Node::has()
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	const ComponentID componentID = generateComponentID<T>();
	auto it = m_componentIDs.find(componentID);
	return it != m_componentIDs.end();
}

template<typename T>
inline void Node::setDirty()
{
	m_dirtyComponent.insert(generateComponentID<T>());
}

}