#pragma once

#include <Aka/Core/Container/HashMap.hpp>
#include <Aka/Scene/ComponentAllocator.hpp>
#include <Aka/Scene/Node.hpp>
#include <Aka/Memory/Pool.h>
#include <Aka/Core/Config.h>

namespace aka {

class NodeAllocator
{
public:
	NodeAllocator();
	NodeAllocator(const NodeAllocator&) = delete;
	NodeAllocator(NodeAllocator&&) = delete;
	NodeAllocator& operator=(const NodeAllocator&) = delete;
	NodeAllocator& operator=(NodeAllocator&&) = delete;
	~NodeAllocator();

public: // Components
	// Allocate component of underlying type
	template <typename C> C* allocate(Node* _node);
	// Allocate component of given type
	ComponentBase* allocate(ComponentID _componentID, Node* _node);
	// Deallocate component of any type
	void deallocate(ComponentBase* component);

public: // Nodes
	// Allocate a node from pool
	Node* create(const char* _name);
	// Deallocate a node from pool
	void destroy(Node* _node);
public:
	PoolIterator<Node> begin() { return m_nodePool.begin(); }
	PoolIterator<Node> end() { return m_nodePool.end(); }
	template <typename C> PoolRange<C> components();
	void visitNodes(std::function<void(Node&)> _callback);
	void visitComponentPools(std::function<void(ComponentBase&)> _callback);
	void visitComponentPool(ComponentID _componentID, std::function<void(ComponentBase&)> _callback);
private:
	ComponentAllocatorMap m_componentMap;
	Pool<Node> m_nodePool;
};

template <typename C> C* NodeAllocator::allocate(Node* _node) {
	static_assert(std::is_base_of<ComponentBase, C>::value);
	return reinterpret_cast<C*>(allocate(Component<C, typename C::Archive>::getComponentID(), _node));
}

template<typename C>
inline PoolRange<C> NodeAllocator::components()
{
	ComponentAllocator<C>* allocator = reinterpret_cast<ComponentAllocator<C>*>(m_componentMap.get(Component<C, typename C::Archive>::getComponentID()));
	return PoolRange<C>(allocator->begin(), allocator->end());
}

};