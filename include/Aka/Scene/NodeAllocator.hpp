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

	// Allocate component of underlying type
	template <typename C> C* allocate(Node* _node);
	// Allocate component of given type
	ComponentBase* allocate(ComponentID _componentID, Node* _node);
	// Deallocate component of any type
	void deallocate(ComponentBase* component);

	// Allocate a node from pool
	Node* create(const char* _name);
	// Deallocate a node from pool
	void destroy(Node* _node);
	// Get node count
	size_t getAllocatedNodeCount() const { return m_nodePool.count(); }
private:
	ComponentAllocatorMap m_componentMap;
	Pool<Node> m_nodePool;
};

template <typename C> C* NodeAllocator::allocate(Node* _node) {
	static_assert(std::is_base_of<ComponentBase, C>::value);
	return reinterpret_cast<C*>(allocate(Component<C, typename C::Archive>::getComponentID(), _node));
}

};