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
	ComponentAllocatorBase(ComponentID _componentID, const char* _name) : m_componentID(_componentID), m_name(_name) {}
	virtual ~ComponentAllocatorBase() {}

	ComponentID getComponentID() const { return m_componentID; }
	virtual ComponentAllocatorBase* clone() const = 0;

	virtual ComponentBase* allocateBase(Node* _node) = 0;
	virtual void deallocateBase(ComponentBase* _component) = 0;
	virtual void visitPool(std::function<void(ComponentBase&)> _callback) = 0;

	const char* getName() const { return m_name;  }
private:
	ComponentID m_componentID;
	const char* m_name;
};

template<typename T>
class ComponentAllocator : public ComponentAllocatorBase
{
	static_assert(std::is_base_of<ComponentBase, T>::value);
public:
	ComponentAllocator(ComponentID _componentID, const char* _name) :
		ComponentAllocatorBase(_componentID, _name)
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
		return new ComponentAllocator<T>(getComponentID(), getName());
	}
	PoolIterator<T> begin() { return m_pool.begin(); }
	PoolIterator<T> end() { return m_pool.end(); }
protected:
	ComponentBase* allocateBase(Node* _node) override
	{
		return allocate(_node);
	}
	void deallocateBase(ComponentBase* _component) override
	{
		deallocate(reinterpret_cast<T*>(_component));
	}
	void visitPool(std::function<void(ComponentBase&)> _callback) override {
		for (T& component : m_pool) {
			_callback(component);
		}
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
	void visit(std::function<void(ComponentAllocatorBase*)> _callback);
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
	ComponentRegister(ComponentID _componentID, const char* _name) {
		getDefaultComponentAllocators().add(_componentID, new ComponentAllocator<T>(_componentID, _name));
	}
};

};