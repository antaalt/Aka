#pragma once

#include <Aka/Core/Container/String.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/Memory/Pool.h>

#include <Aka/Scene/Node.hpp>

namespace aka {

class Node2D : public Node
{
public:
	Node2D();
	Node2D(const char* name);
	~Node2D();

public:
	// Add a node to this node.
	Node2D* add(const char* name);
	// Remove a node from this node.
	void remove(Node2D* entity);
public:
	void create(gfx::GraphicDevice* _device);
	void destroy(gfx::GraphicDevice* _device);
	void update(Time deltaTime);
	void fixedUpdate(Time deltaTime);
	void prepare(gfx::GraphicDevice* _device) override;
	void render(gfx::GraphicDevice* _device, gfx::Frame* _frame);
	void finish(gfx::GraphicDevice* _device) override;

private: // Hierarchy
	Node2D* m_parent;
	Vector<Node2D*> m_childrens;
};

}