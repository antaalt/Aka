#include <Aka/Scene/Node.hpp>

namespace aka {

Node::Node() : 
	m_parent(nullptr),
	m_name("Unknown"),
	m_updateFlags(NodeUpdateFlag::None),
	m_localTransform(mat4f::identity())
{
}
Node::Node(const char* name) : 
	m_parent(nullptr),
	m_name(name),
	m_updateFlags(NodeUpdateFlag::None),
	m_localTransform(mat4f::identity())
{
}
Node::~Node()
{
	AKA_ASSERT(m_componentsToActivate.size() == 0, "Missing components");
	AKA_ASSERT(m_componentsActive.size() == 0, "Missing components");
	AKA_ASSERT(m_componentsToDeactivate.size() == 0, "Missing components");
}

void Node::create(AssetLibrary* library, Renderer* renderer)
{
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
	for (Component* component : m_componentsToActivate)
	{
		component->detach();
		delete component;
	}
	for (Component* component : m_componentsActive)
	{
		component->deactivate(library, renderer);
		component->detach();
		delete component;
	}
	for (Component* component : m_componentsToDeactivate)
	{
		component->deactivate(library, renderer);
		component->detach();
		delete component;
	}
	m_componentsActiveMask = 0;
	m_componentsToActivateMask = 0;
	m_componentsToDeactivateMask = 0;
	m_componentsActiveMask = 0;
	m_componentsToActivate.clear();
	m_componentsActive.clear();
	m_componentsToDeactivate.clear();
}

void Node::update(AssetLibrary* library, Renderer* renderer)
{
	/*uint32_t mask = toMask(m_updateFlags);
	uint32_t index = 0;
	while ((index = firstbitlow(mask)) != 0)
	{
		NodeUpdateFlag flag = NodeUpdateFlag(1U << index);
		index &= ~(1U << index);
		switch (flag)
		{
		case NodeUpdateFlag::Transform:
			break;
		case NodeUpdateFlag::ComponentDirtyBase:
			break;
		}
	}*/
	if (m_componentsActiveMask & (1ULL << (uint64_t)Component::generateID<StaticMeshComponent>()))
	{
		// For now, hard update static mesh component
		get<StaticMeshComponent>().setInstanceTransform(getWorldTransform());
	}

	// Update children
	for (Node* childrens : m_childrens)
	{
		childrens->update(library, renderer);
	}
	// Activate components
	for (Component*& component : m_componentsToActivate)
	{
		const uint64_t mask = 1ULL << static_cast<uint64_t>(component->id());
		component->activate(library, renderer);
		m_componentsActive.append(component);
		m_componentsToActivateMask &= ~mask;
		m_componentsActiveMask |= mask;
	}
	m_componentsToActivate.clear();
	// Deactivate components
	for (Component*& component : m_componentsToDeactivate)
	{
		const uint64_t mask = 1ULL << static_cast<uint64_t>(component->id());
		component->deactivate(library, renderer);
		component->detach();
		m_componentsToDeactivateMask &= ~mask;
		delete component;
	}
	m_componentsToDeactivate.clear();
}

void Node::update(Time deltaTime)
{
}

void Node::fixedUpdate(Time deltaTime)
{
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
	AKA_ASSERT(child != this, "Trying to add itself as child");
	AKA_ASSERT(child->m_parent == nullptr, "Child already have a parent");
	child->m_parent = this;
	m_childrens.append(child);
}
void Node::removeChild(Node* child)
{
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

mat4f& Node::getLocalTransform()
{
	return m_localTransform;
}
const mat4f& Node::getLocalTransform() const
{
	return m_localTransform;
}
mat4f Node::getWorldTransform() const
{
	// TODO should check its a node3D, or move all to Node ?
	if (getParent())
		return getParent()->getWorldTransform() * m_localTransform;
	return m_localTransform;
}
void Node::setLocalTransform(const mat4f& transform)
{
	m_localTransform = transform;
}

};