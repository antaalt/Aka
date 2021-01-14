#pragma once

#include "GraphicBackend.h"
#include "Component.h"
#include "Entity.h"

#include <vector>
#include <set>

namespace app {

class World;

// Physic
// IA
// Render
// Son

// Which component do I care about ?
class System
{
public:
	friend class World;
	// Add entities to the system
	void add(Entity* entity);
	// Remove entities to the system
	void remove(Entity* entity);
	// Check an entity has all required component by the system
	virtual bool valid(Entity* entity);
public:
	virtual void create();
	virtual void destroy();

	virtual void update();
	virtual void render(GraphicBackend &backend);
protected:
	std::set<Entity*> m_entities;
};

}