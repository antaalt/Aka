#include <Aka/Scene/NodeAllocator.hpp>

namespace aka {

NodeAllocator::NodeAllocator() :
	m_nodePool(),
	m_componentMap(getDefaultComponentAllocators())
{
}
NodeAllocator::~NodeAllocator()
{
	AKA_ASSERT(m_nodePool.count() == 0, "Node destroy missing");
	m_nodePool.release([this](Node& node) { Logger::warn(node.getName(), " was not destroyed"); });
}
ComponentBase* NodeAllocator::allocate(ComponentID _componentID, Node* _node)
{
	return m_componentMap.get(_componentID)->allocateBase(_node);
}
void NodeAllocator::deallocate(ComponentBase* _component)
{
	ComponentAllocatorBase* allocator = m_componentMap.get(_component->getComponentID());
	AKA_ASSERT(_component->getComponentID() == allocator->getComponentID(), "Invalid component type");
	return allocator->deallocateBase(_component);
}
Node* NodeAllocator::create(const char* _name)
{
	return m_nodePool.acquire(_name, this);
}
void NodeAllocator::destroy(Node* _node)
{
	return m_nodePool.release(_node);
}
void NodeAllocator::visitNodes(std::function<void(Node&)> _callback) {
	for (Node& node : m_nodePool)
	{
		_callback(node);
	}
}
void NodeAllocator::visitComponentPools(std::function<void(ComponentBase&)> _callback) {
	m_componentMap.visit([=](ComponentAllocatorBase* _componentAllocator) {
		_componentAllocator->visitPool([=](ComponentBase& _component) {
			_callback(_component);
			});
		});
}
void NodeAllocator::visitComponentPool(ComponentID _componentID, std::function<void(ComponentBase&)> _callback) {
	ComponentAllocatorBase* allocator = m_componentMap.get(_componentID);
	allocator->visitPool(_callback);
}

}