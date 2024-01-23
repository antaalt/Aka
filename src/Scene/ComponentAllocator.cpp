#include <Aka/Scene/ComponentAllocator.hpp>
#include <Aka/Memory/Allocator.h>

namespace aka {

ComponentAllocatorMap::ComponentAllocatorMap(Allocator& _allocator) :
	m_allocator(_allocator)
{
}
ComponentAllocatorMap::ComponentAllocatorMap(const ComponentAllocatorMap& _copy) :
	m_allocator(_copy.m_allocator)
{
	*this = _copy;
}
ComponentAllocatorMap::ComponentAllocatorMap(ComponentAllocatorMap&& _copy) :
	m_allocator(std::move(_copy.m_allocator))
{
	*this = std::move(_copy);
}
ComponentAllocatorMap& ComponentAllocatorMap::operator=(const ComponentAllocatorMap& _copy)
{
	for (const auto& component : _copy.m_container)
	{
		m_container.insert(std::make_pair(component.first, component.second->clone()));
	}
	return *this;
}
ComponentAllocatorMap& ComponentAllocatorMap::operator=(ComponentAllocatorMap&& _copy)
{
	std::swap(m_container, _copy.m_container);
	return *this;
}
ComponentAllocatorMap::~ComponentAllocatorMap()
{
	for (const auto& component : m_container)
	{
		delete component.second;
	}
}

void ComponentAllocatorMap::add(ComponentID _componentID, ComponentAllocatorBase* _componentAllocator)
{
	std::cout << "Registering component " << _componentAllocator->getName() << std::endl;
	m_container.insert(std::make_pair(_componentID, _componentAllocator));
}
ComponentAllocatorBase* ComponentAllocatorMap::get(ComponentID _componentID)
{
	AKA_ASSERT(m_container.find(_componentID) != m_container.end(), "Component does not exist");
	return m_container[_componentID];
}
void ComponentAllocatorMap::visit(std::function<void(ComponentAllocatorBase*)> _callback)
{
	for (auto& pair : m_container)
	{
		_callback(pair.second);
	}
}
ComponentAllocatorMap& getDefaultComponentAllocators() {
	// Pass allocator to ComponentAllocator so that it outlives it & avoid a crash because allocator is used in pool destructor but it was already destroyed.
	static ComponentAllocatorMap s_defaultComponentAllocators(mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default));
	return s_defaultComponentAllocators;
}

}