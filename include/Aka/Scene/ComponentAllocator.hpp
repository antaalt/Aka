#pragma once 

#include <Aka/Core/Container/HashMap.hpp>
#include <Aka/Scene/ComponentType.hpp>
#include <Aka/Memory/Pool.h>

namespace aka {

class Node;
class ComponentBase;

class ComponentAllocatorBase
{
public:
	ComponentAllocatorBase(ComponentID _componentID) : m_componentID(_componentID) {}
	virtual ~ComponentAllocatorBase() {}

	ComponentID getComponentID() const { return m_componentID; }
	virtual ComponentAllocatorBase* clone() const = 0;

	virtual ComponentBase* allocateBase(Node* _node) = 0;
	virtual void deallocateBase(ComponentBase* _component) = 0;
private:
	ComponentID m_componentID;
};

template<typename T>
class ComponentAllocator : public ComponentAllocatorBase
{
	static_assert(std::is_base_of<ComponentBase, T>::value);
public:
	ComponentAllocator(ComponentID _componentID) :
		ComponentAllocatorBase(_componentID)
	{
	}
	T* allocate(Node* _node)
	{
		return m_pool.acquire(_node);
	}
	void deallocate(T* _component)
	{
		m_pool.release(_component);
	}
	ComponentAllocatorBase* clone() const override {
		return new ComponentAllocator<T>(getComponentID());
	}
protected:
	ComponentBase* allocateBase(Node* _node) override
	{
		return allocate(_node);
	}
	void deallocateBase(ComponentBase* _component) override
	{
		deallocate(reinterpret_cast<T*>(_component));
	}
private:
	Pool<T> m_pool;
};

class ComponentAllocatorMap
{
public:
	ComponentAllocatorMap(Allocator& _allocator);
	ComponentAllocatorMap(const ComponentAllocatorMap& _copy);
	ComponentAllocatorMap(ComponentAllocatorMap&& _copy);
	ComponentAllocatorMap& operator=(const ComponentAllocatorMap& _copy);
	ComponentAllocatorMap& operator=(ComponentAllocatorMap&& _copy);
	~ComponentAllocatorMap();

	void add(ComponentID _componentID, ComponentAllocatorBase* _componentAllocator);
	ComponentAllocatorBase* get(ComponentID _componentID);
private:
	Allocator& m_allocator;
	HashMap<ComponentID, ComponentAllocatorBase*> m_container;
};

// Get the prototype of all component allocator map
ComponentAllocatorMap& getDefaultComponentAllocators();

// Self instancing helper for component allocator
template <typename T>
class ComponentRegister {
public:
	ComponentRegister(ComponentID _componentID) {
		getDefaultComponentAllocators().add(_componentID, new ComponentAllocator<T>(_componentID));
	}
};

};