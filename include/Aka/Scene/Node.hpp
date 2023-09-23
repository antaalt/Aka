#pragma once

#include <Aka/Core/Container/String.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/Memory/Pool.h>

#include <Aka/Scene/Component.hpp>
#include <Aka/Scene/Component/CameraComponent.hpp>
#include <Aka/Scene/Component/StaticMeshComponent.hpp>

namespace aka {

// TODO this class would need to be extendable for each possible custom type we can get create..
struct ComponentPool
{
	template <typename T>
	Pool<T>& get();
private:
	Pool<StaticMeshComponent> m_staticMeshes;
	Pool<CameraComponent> m_cameras; // Attributed to views ? do we need shadow camera ?
};

template <>
inline Pool<StaticMeshComponent>& ComponentPool::get() {
	return m_staticMeshes;
}
template <>
inline Pool<CameraComponent>& ComponentPool::get() {
	return m_cameras;
}

class Node
{
public:
	Node() : m_name("Unknown"), m_pools(new ComponentPool) {}
	Node(const char* name) : m_name(name), m_pools(new ComponentPool) {}
	virtual ~Node() { delete m_pools; } // TODO release components.

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
	void setDirty(ComponentType type) { m_dirtyMask |= ComponentTypeMask(1U << EnumToIndex(type)); }
private: // Data
	String m_name;
	Vector<Component*> m_components;
private: // Allocation
	ComponentPool* m_pools;
private:
	ComponentTypeMask m_dirtyMask;
};

template<typename T, typename ...Args>
inline T& Node::attach(Args && ...args)
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	for (Component* attachedComponent : m_components)
	{
		if (typeid(*attachedComponent) == typeid(T))
		{
			AKA_ASSERT(false, "Trying to attach alredy attached component");
			AKA_CRASH();
		}
	}
	Pool<T>& pool = m_pools->get<T>();
	T* component = pool.acquire(std::forward<Args>(args)...);
	component->onAttach();
	return reinterpret_cast<T&>(*m_components.append(component));
}

template<typename T>
inline void Node::detach()
{
	static_assert(std::is_base_of<Component, T>::value, "Invalid type");
	for (Component*& attachedComponent : m_components)
	{
		if (typeid(*attachedComponent) == typeid(T))
		{
			// Detach
			attachedComponent->onDetach();
			m_components.remove(&attachedComponent);
			// Release
			Pool<T>& pool = m_pools->get<T>();
			pool.release(reinterpret_cast<T*>(attachedComponent));
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
		if (typeid(*attachedComponent) == typeid(T))
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
		// TODO should not use RTTI
		if (typeid(*attachedComponent) == typeid(T))
		{
			return true;
		}
	}
	return false;
}

}