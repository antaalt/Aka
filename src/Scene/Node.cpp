#include <Aka/Scene/Node.hpp>

#include <Aka/Scene/Component/StaticMeshComponent.hpp>
#include <Aka/Scene/NodeAllocator.hpp>

namespace aka {

Node::Node(NodeAllocator* _allocator) :
	m_parent(nullptr),
	m_name("Unknown"),
	m_allocator(_allocator),
	m_updateFlags(NodeUpdateFlag::None),
	m_localTransform(mat4f::identity()),
	m_cacheWorldTransform(mat4f::identity())
{
}
Node::Node(const char* name, NodeAllocator* _allocator) :
	m_parent(nullptr),
	m_name(name),
	m_allocator(_allocator),
	m_updateFlags(NodeUpdateFlag::None),
	m_localTransform(mat4f::identity()),
	m_cacheWorldTransform(mat4f::identity())
{
}
Node::~Node()
{
	AKA_ASSERT(m_componentsToActivate.size() == 0, "Missing components");
	AKA_ASSERT(m_componentsActive.size() == 0, "Missing components");
	AKA_ASSERT(m_componentsToDeactivate.size() == 0, "Missing components");
}

void Node::attach(ComponentBase* component)
{
	const ComponentID id = component->getComponentID();
	AKA_ASSERT(m_componentIDs.find(id) == m_componentIDs.end(), "Trying to attach non attached component");
	m_componentIDs.insert(id);
	component->onAttach();
	m_componentsToActivate.insert(std::make_pair(id, component));
}

void Node::create(AssetLibrary* library, Renderer* renderer)
{
	for (Node* childrens : m_childrens)
	{
		childrens->create(library, renderer);
	}
}

void Node::destroy(AssetLibrary* library, Renderer* renderer)
{
	// Destroy childs.
	for (Node* childrens : m_childrens)
	{
		// This will call remove child on this.
		childrens->destroy(library, renderer);
	}
	// Destroy components
	for (std::pair<ComponentID, ComponentBase*> component : m_componentsToActivate)
	{
		component.second->detach();
		m_allocator->deallocate(component.second);
	}
	for (std::pair<ComponentID, ComponentBase*> component : m_componentsActive)
	{
		component.second->deactivate(library, renderer);
		component.second->detach();
		m_allocator->deallocate(component.second);
	}
	for (std::pair<ComponentID, ComponentBase*> component : m_componentsToDeactivate)
	{
		component.second->deactivate(library, renderer);
		component.second->detach();
		m_allocator->deallocate(component.second);
	}
	m_componentIDs.clear();
	m_componentsToActivate.clear();
	m_componentsActive.clear();
	m_componentsToDeactivate.clear();
}

void Node::updateComponentLifecycle(AssetLibrary* library, Renderer* renderer)
{
	{ // Components lifecycle.
		// Activate components
		for (std::pair<ComponentID, ComponentBase*> component : m_componentsToActivate)
		{
			component.second->activate(library, renderer);
			m_componentsActive.insert(component);
		}
		m_componentsToActivate.clear();
		// Deactivate components
		for (std::pair<ComponentID, ComponentBase*> component : m_componentsToDeactivate)
		{
			component.second->deactivate(library, renderer);
			component.second->detach();
			m_allocator->deallocate(component.second);
		}
		m_componentsToDeactivate.clear();
	}
}

void Node::prepareUpdate()
{
	if (asBool(NodeUpdateFlag::TransformDirty & m_updateFlags))
	{
		m_cacheWorldTransform = computeWorldTransform();
		m_updateFlags &= ~NodeUpdateFlag::TransformDirty;
		visitAllChildrens([this](Node* node) {
			node->m_cacheWorldTransform = node->computeWorldTransform();
		});
		for (ComponentMap::value_type& component : m_componentsActive)
		{
			component.second->transformUpdate();
		}
	}
	else if (asBool(NodeUpdateFlag::HierarchyUpdated & m_updateFlags))
	{
		m_cacheWorldTransform = computeWorldTransform();
		for (ComponentMap::value_type& component : m_componentsActive)
		{
			component.second->hierarchyUpdate();
		}
	}
}

void Node::finishUpdate()
{
	// Remove flag after so that child can check if this flag is set
	m_updateFlags &= ~NodeUpdateFlag::TransformUpdated;
	m_updateFlags &= ~NodeUpdateFlag::HierarchyUpdated;
}

void Node::unlink()
{
	AKA_ASSERT(m_parent, "No parent");
	Node* parent = m_parent;
	parent->removeChild(this);
	for (Node* child : m_childrens)
	{
		parent->m_childrens.append(child);
	}
	m_childrens.clear();
}

void Node::addChild(Node* child)
{
	setUpdateFlag(NodeUpdateFlag::HierarchyUpdated, false); // Do not propagate update as adding a child do not affect other childs.
	AKA_ASSERT(child != this, "Trying to add itself as child");
	AKA_ASSERT(child->m_parent == nullptr, "Child already have a parent");
	child->m_parent = this;
	m_childrens.append(child);
}
void Node::removeChild(Node* child)
{
	setUpdateFlag(NodeUpdateFlag::HierarchyUpdated, false); // Do not propagate update as removing a child do not affect other childs.
	AKA_ASSERT(child != this, "Trying to remove itself as child");
	auto it = std::find(m_childrens.begin(), m_childrens.end(), child);
	if (it != m_childrens.end())
	{
		m_childrens.remove(it);
		child->m_parent = nullptr;
	}
	else
	{
		AKA_ASSERT(false, "Not found.");
	}
}
void Node::setParent(Node* parent)
{
	setUpdateFlag(NodeUpdateFlag::HierarchyUpdated | NodeUpdateFlag::TransformUpdated | NodeUpdateFlag::TransformDirty);
	if (m_parent)
		m_parent->removeChild(this);
	m_parent = parent;
	m_parent->m_childrens.append(this);
}
Node* Node::getParent()
{
	return m_parent;
}
const Node* Node::getParent() const
{
	return m_parent;
}
uint32_t Node::getChildCount() const
{
	return (uint32_t)m_childrens.size();
}
Node* Node::getChild(uint32_t iChild)
{
	return m_childrens[iChild];
}
const Node* Node::getChild(uint32_t iChild) const
{
	return m_childrens[iChild];
}

void Node::visitChildrens(std::function<void(Node*)> _callback)
{
	for (Node* child : m_childrens) 
	{
		_callback(child);
	}
}
void Node::visitAllChildrens(std::function<void(Node*)> _callback)
{
	for (Node* child : m_childrens)
	{
		_callback(child);
		// Recurse childrens.
		child->visitAllChildrens(_callback);
	}
}

const mat4f& Node::getLocalTransform() const
{
	return m_localTransform;
}
const mat4f& Node::getWorldTransform() const
{
	AKA_ASSERT(!asBool(NodeUpdateFlag::TransformDirty & m_updateFlags), "Getting dirty world transform. Should call computeWorldTransform instead here as it was updated and not cached yet.");
	return m_cacheWorldTransform;
}
mat4f Node::getParentTransform() const
{
	if (getParent())
		return getParent()->getWorldTransform();
	return mat4f::identity();
}
void Node::setLocalTransform(const mat4f& _transform, bool _computeWorld)
{
	m_localTransform = _transform;
	if (_computeWorld)
	{
		m_cacheWorldTransform = computeWorldTransform();
		visitAllChildrens([this](Node* node) {
			node->m_cacheWorldTransform = node->computeWorldTransform();
			node->setUpdateFlag(NodeUpdateFlag::TransformUpdated);
		});
		setUpdateFlag(NodeUpdateFlag::TransformUpdated);
	}
	else
	{
		// Delay update.
		setUpdateFlag(NodeUpdateFlag::TransformUpdated | NodeUpdateFlag::TransformDirty);
		visitAllChildrens([this](Node* node) {
			node->setUpdateFlag(NodeUpdateFlag::TransformUpdated | NodeUpdateFlag::TransformDirty);
		});
	}
}
mat4f Node::computeWorldTransform() const
{
	if (getParent())
		return getParent()->computeWorldTransform() * m_localTransform;
	return m_localTransform;
}

void Node::setUpdateFlag(NodeUpdateFlag flag, bool recurse)
{
	m_updateFlags |= flag;
	if (recurse)
	{
		for (Node* child : m_childrens)
			child->setUpdateFlag(flag);
	}
}

};