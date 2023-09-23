#pragma once

#include <Aka/Core/Container/String.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/Memory/Pool.h>

#include <Aka/Scene/Node.hpp>

namespace aka {

class Node3D : public Node
{
public:
	Node3D();
	Node3D(const char* name);
	~Node3D();

public:
	// Add a node to this node.
	void addChild(Node3D* node);
	// Remove a node from this node.
	void removeChild(Node3D* node);
public:
	void create(gfx::GraphicDevice* _device) override;
	void destroy(gfx::GraphicDevice* _device) override;
	void update(Time deltaTime) override;
	void fixedUpdate(Time deltaTime) override;

	// Dont need this, we dont draw from here, renderer handle this with different rendering subsystem. Only engine logic in node, with move & updates.
	void prepare(gfx::GraphicDevice* _device) override {}
	void render(gfx::GraphicDevice* _device, gfx::Frame* _frame) {}
	void finish(gfx::GraphicDevice* _device) override {}
public:
	void setLocalTransform(const mat4f& transform);
	mat4f& getLocalTransform();
	const mat4f& getLocalTransform() const;
	mat4f getWorldTransform() const;
public:
	void setParent(Node3D* parent);
	Node3D* getParent();
	uint32_t getChildCount() const;
	Node3D* getChild(uint32_t iChild);
private: // Hierarchy
	Node3D* m_parent;
	Vector<Node3D*> m_childrens;
private:
	mat4f m_localTransform;
};

}