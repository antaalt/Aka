#pragma once

#include <Aka/OS/Time.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/Core/Geometry.h>

namespace aka {

class Component
{
public:
	virtual void onUpdate(Time deltaTime) {}
	virtual void onFixedUpdate(Time deltaTime) {}
	virtual void onRender(Time deltaTime) {}
};

class TransformComponent : Component
{
	mat4f transform;
};

class StaticMeshComponent : Component
{
	
};

class Entity
{
public:
	// Get a component
	template <typename T> T* get() { static_assert(std::is_base_of<T, Component>::value, "Not a component"); return m_components[0]; }
	// Check for a component
	template <typename T> bool has() { return true; }
	// Add a component to entity
	template <typename T> void add();
	// Remove a component from entity
	template <typename T> void remove();
private:
	//World* m_world;
	Vector<Component*> m_components;
};

class World
{
public:
	// Entity
	Entity* create() { return new Entity; } // TODO entity pool
	void destroy(Entity* entity) { delete entity; }
	
	// Component
	template <typename T> void attach(Entity* entity);
	template <typename T> void detach(Entity* entity);

	// Action
	void update(Time deltaTime);
	void fixedUpdate(Time deltaTime);
	void render(CommandList* commandList);
private:
	
	Vector<Component*> pools[5];
};




};