#include "Entity.h"

#include "World.h"

namespace aka {

Entity::Entity(World* world) :
	m_alive(true),
	m_world(world)
{
}

void Entity::destroy()
{
	m_alive = false;
}

};


