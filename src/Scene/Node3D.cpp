#include <Aka/Scene/Node3D.hpp>

namespace aka {

Node3D::Node3D() :
	Node("Unknown"),
	m_parent(nullptr)
{
}
Node3D::Node3D(const char* name) : 
	Node(name),
	m_parent(nullptr)
{
}
Node3D::~Node3D()
{
}

Node3D* Node3D::addChild(const char* name)
{
	return m_childrens.append(new Node3D(name));
}
void Node3D::removeChild(Node3D* entity)
{
	auto it = std::find(m_childrens.begin(), m_childrens.end(), entity);
	if (it != m_childrens.end())
	{
		m_childrens.remove(it);
		// TODO delete here or do not new up ?
		AKA_ASSERT(false, "");
	}
}
void Node3D::create(gfx::GraphicDevice* _device)
{
	// Should rename this, create does not mean much about lifetime.
	// Create work on system that are created at start and finish, but does not mean much for variable lifetime.
	// Should have
	// onCreation()
	// onBecomeActive(Renderer* renderer)
	// 
	// Node live happily until someone ask for destroy.
	// We mark it via a flag somehow and destoy it later ? cuz we need to call callback & all
	// 
	// onBecomeInactive(Renderer* renderer)
	// onDestruction()
}
void Node3D::destroy(gfx::GraphicDevice* _device)
{
	if (m_parent)
	{
		m_parent->removeChild(this);
	}
	for (Node3D* childrens : m_childrens)
	{
		// This will call remove child on this.
		childrens->destroy(_device);
	}
}
void Node3D::update(Time deltaTime)
{

}
void Node3D::fixedUpdate(Time deltaTime)
{

}
void Node3D::render(gfx::GraphicDevice* _device, gfx::Frame* _frame)
{

}
mat4f& Node3D::getLocalTransform()
{
	return m_localTransform;
}
const mat4f& Node3D::getLocalTransform() const
{
	return m_localTransform;
}
mat4f Node3D::getWorldTransform() const
{
	if (m_parent)
		return m_parent->getWorldTransform() * m_localTransform;
	return m_localTransform;
}
void Node3D::setLocalTransform(const mat4f& transform)
{
	m_localTransform = transform;
}
void Node3D::setParent(Node3D* parent)
{
	m_parent = parent;
}
Node3D* Node3D::getParent()
{
	return m_parent;
}
uint32_t Node3D::getChildCount() const
{
	return (uint32_t)m_childrens.size();
}
Node3D* Node3D::getChild(uint32_t iChild)
{
	return m_childrens[iChild];
}

};


