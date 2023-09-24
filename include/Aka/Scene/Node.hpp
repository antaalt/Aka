#pragma once

#include <Aka/Core/Container/String.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/Memory/Pool.h>

#include <Aka/Scene/Component.hpp>
#include <Aka/Scene/Component/CameraComponent.hpp>
#include <Aka/Scene/Component/StaticMeshComponent.hpp>

namespace aka {

enum class NodeUpdateFlag : uint64_t
{
	None				= 0,

	Transform			= 1 << 0,
	ComponentDirtyBase	= 1 << 1, // Need to be the last component
	// Do not add flags after this, leave space for component dirty
	// Their value is ComponentDirtyBase << Component::generateID<MyComponent>()
};
AKA_IMPLEMENT_BITMASK_OPERATOR(NodeUpdateFlag);

class Node
{
public:
	Node();
	Node(const char* name);
	virtual ~Node();

	// Attach a component to the entity
	template <typename T, typename... Args> T& attach(Args&&... args);
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
	const String& getName() const { return m_name; }
	bool isOrphan() const { return m_componentsActive.size() == 0; }
	uint32_t getComponentCount() const { return (uint32_t)m_componentsActive.size(); }
	template<typename T>
	void setDirty();
	void setFlag(NodeUpdateFlag flag) { m_updateFlags |= flag; }
public:
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
	void setLocalTransform(const mat4f& transform);
	mat4f& getLocalTransform();
	const mat4f& getLocalTransform() const;
	mat4f getWorldTransform() const;
private:
	mat4f m_localTransform;
private: // Data
	String m_name;
	// TODO use map, limit of 64 components here :( (check NodeUpdateFlag also)
	uint64_t m_componentsToActivateMask = 0;	// Mask of all component pending activation.
	uint64_t m_componentsActiveMask = 0;		// Mask of all active component.
	uint64_t m_componentsToDeactivateMask = 0; // Mask of all component pending deactivation.
	uint64_t componentMask() const { return m_componentsToActivateMask | m_componentsActiveMask | m_componentsToDeactivateMask; }
	Vector<Component*> m_componentsActive;
	Vector<Component*> m_componentsToActivate;
	Vector<Component*> m_componentsToDeactivate;
private:
	NodeUpdateFlag m_updateFlags;
};

template<typename T, typename ...Args>
inline T& Node::attach(Args && ...args)
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	ComponentID id = Component::generateID<T>();
	uint32_t mask = 1U << static_cast<uint32_t>(id);
	if (componentMask() & mask)
	{
		AKA_ASSERT(false, "Trying to attach alredy attached component");
		AKA_CRASH();
	}
	m_componentsToActivateMask |= mask;
	T* component = new T(std::forward<Args>(args)...);
	component->onAttach();
	return reinterpret_cast<T&>(*m_componentsToActivate.append(component));
}

template<typename T>
inline void Node::detach()
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	const ComponentID componentID = Component::generateID<T>();
	const uint64_t mask = 1ULL << static_cast<uint64_t>(componentID);
	if (!(componentMask() & mask))
	{
		AKA_ASSERT(false, "Trying to detach non attached component");
		AKA_CRASH();
	}
	auto findComponent = [](Vector<Component*>& components, ComponentID id) -> Component*& {
		for (Component*& component : components)
		{
			if (component->id() == id)
			{
				return component;
			}
		}
		AKA_UNREACHABLE;
		return components[0];
	};
	if (m_componentsActiveMask & mask)
	{
		if (Component*& component = findComponent(m_componentsActive, componentID))
		{
			m_componentsActiveMask &= ~mask;
			m_componentsToDeactivateMask |= mask;
			m_componentsActive.remove(&component);
			// Component still active until deactivate was call on it.
			m_componentsToDeactivate.append(component);
			return;
		}
	}
	else if (m_componentsToActivateMask & mask)
	{
		if (Component*& component = findComponent(m_componentsToActivate, componentID))
		{
			m_componentsToActivateMask &= ~mask;
			m_componentsToDeactivateMask |= mask;
			m_componentsToActivate.remove(&component);
			// Component still active until deactivate was call on it.
			m_componentsToDeactivate.append(component);
			return;
		}
	}
	else if (m_componentsToDeactivateMask & mask)
	{
		AKA_ASSERT(false, "Trying to detach already detached component");
	}
	else
	{
		AKA_ASSERT(false, "Trying to detach non attached component");
		AKA_CRASH();
	}
	AKA_UNREACHABLE;
}

template<typename T>
inline T& Node::get()
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	const ComponentID componentID = Component::generateID<T>();
	const uint32_t mask = 1U << static_cast<uint32_t>(componentID);
	auto findComponent = [](Vector<Component*>& components, ComponentID id) -> Component* {
		for (Component*& component : components)
		{
			if (component->id() == id)
			{
				return component;
			}
		}
		return nullptr;
	};
	if (mask & m_componentsToActivateMask)
	{
		return *reinterpret_cast<T*>(findComponent(m_componentsToActivate, componentID));
	}
	else if (mask & m_componentsActiveMask)
	{
		return *reinterpret_cast<T*>(findComponent(m_componentsActive, componentID));
	}
	else if (mask & m_componentsToDeactivateMask)
	{
		return *reinterpret_cast<T*>(findComponent(m_componentsToDeactivate, componentID));
	}
	AKA_ASSERT(false, "Trying to get unattached component");
	AKA_CRASH();
}
template<typename T>
inline bool aka::Node::has()
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	const ComponentID componentID = Component::generateID<T>();
	const uint32_t mask = 1U << static_cast<uint32_t>(componentID);
	return (mask & m_componentsActiveMask) || (mask & m_componentsToActivateMask);
}

template<typename T>
inline void Node::setDirty()
{
	m_updateFlags |= static_cast<NodeUpdateFlag>(static_cast<uint32_t>(NodeUpdateFlag::ComponentDirtyBase) << static_cast<uint32_t>(Component::generateID<T>()));
}

}