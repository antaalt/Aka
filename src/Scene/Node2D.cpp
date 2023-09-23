#include <Aka/Scene/Node2D.hpp>

namespace aka {

Node2D::Node2D() :
	Node("Unknown"),
	m_parent(nullptr)
{
}
Node2D::Node2D(const char* name) :
	Node(name),
	m_parent(nullptr)
{
}
Node2D::~Node2D()
{
}

Node2D* Node2D::add(const char* name)
{
	return m_childrens.append(new Node2D(name));
}
void Node2D::remove(Node2D* entity)
{
	// TODO
}
void Node2D::create(gfx::GraphicDevice* _device)
{

}
void Node2D::destroy(gfx::GraphicDevice* _device)
{

}
void Node2D::update(Time deltaTime)
{

}
void Node2D::fixedUpdate(Time deltaTime)
{

}
void Node2D::render(gfx::GraphicDevice* _device, gfx::Frame* _frame)
{

}
};


