#pragma once

#include <Aka/Core/Container/String.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/Memory/Pool.h>

#include <Aka/Scene/Component.hpp>
#include <Aka/Scene/Component/CameraComponent.hpp>
#include <Aka/Scene/Component/StaticMeshComponent.hpp>

namespace aka {

enum class NodeUpdateFlag : uint32_t
{
	None				= 0,

	Transform			= 1 << 0,
	ComponentDirtyBase	= 1 << 1,
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
	virtual void create(gfx::GraphicDevice* _device) = 0;
	virtual void destroy(gfx::GraphicDevice* _device) = 0;
	virtual void update(Time deltaTime) = 0;
	virtual void fixedUpdate(Time deltaTime) = 0;
	virtual void prepare(gfx::GraphicDevice* _device) = 0;
	virtual void render(gfx::GraphicDevice* _device, gfx::Frame* _frame) = 0;
	virtual void finish(gfx::GraphicDevice* _device) = 0;
public:
	const String& getName() const { return m_name; }
	bool isOrphan() const { return m_components.size() == 0; }
	uint32_t getComponentCount() const { return (uint32_t)m_components.size(); }
	template<typename T>
	void setDirty();
protected:
	void destroyComponents();
private: // Data
	String m_name;
	Vector<Component*> m_components;
private:
	NodeUpdateFlag m_updateFlags;
};

template<typename T, typename ...Args>
inline T& Node::attach(Args && ...args)
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	for (Component* attachedComponent : m_components)
	{
		if (attachedComponent->id() == Component::generateID<T>())
		{
			AKA_ASSERT(false, "Trying to attach alredy attached component");
			AKA_CRASH();
		}
	}
	T* component = new T(std::forward<Args>(args)...);
	component->onAttach();
	return reinterpret_cast<T&>(*m_components.append(component));
}

template<typename T>
inline void Node::detach()
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	for (Component*& attachedComponent : m_components)
	{
		if (attachedComponent->id() == Component::generateID<T>())
		{
			// Detach
			attachedComponent->onDetach();
			m_components.remove(&attachedComponent);
			// Release
			delete attachedComponent;
			return;
		}
	}
	Logger::warn("Trying to detach non attached component");
}

template<typename T>
inline T& Node::get()
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	for (Component* attachedComponent : m_components)
	{
		if (attachedComponent->id() == Component::generateID<T>())
		{
			return reinterpret_cast<T&>(*attachedComponent);
		}
	}
	AKA_ASSERT(false, "Trying to get unattached component");
	AKA_CRASH();
}
template<typename T>
inline bool aka::Node::has()
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	for (Component* attachedComponent : m_components)
	{
		if (attachedComponent->id() == Component::generateID<T>())
		{
			return true;
		}
	}
	return false;
}

template<typename T>
inline void Node::setDirty()
{
	m_updateFlags |= static_cast<NodeUpdateFlag>(static_cast<uint32_t>(NodeUpdateFlag::ComponentDirtyBase) << static_cast<uint32_t>(Component::generateID<T>()));
}

}