#include <Aka/Scene/Node.hpp>

namespace aka {

Node::Node() : 
	m_name("Unknown"),
	m_updateFlags(NodeUpdateFlag::None)
{
}
Node::Node(const char* name) : 
	m_name(name),
	m_updateFlags(NodeUpdateFlag::None)
{
}
Node::~Node()
{
	AKA_ASSERT(m_components.size() == 0, "Missing components");
}

void Node::destroyComponents()
{
	for (Component* component : m_components)
	{
		component->onDetach();
		delete component;
	}
	m_components.clear();
}

};