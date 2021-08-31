#include <Aka/Scene/Entity.h>

#include <Aka/Scene/World.h>

namespace aka {

Entity::Entity() :
	m_handle(entt::null),
	m_world(nullptr)
{
}

Entity::Entity(entt::entity handle, World* world) :
	m_handle(handle),
	m_world(world)
{
}

Entity Entity::null()
{
	return Entity();
}

void Entity::destroy() 
{ 
	m_world->registry().destroy(m_handle); 
}

bool Entity::valid() const
{
	if (m_world == nullptr) return false;
	return m_world->registry().valid(m_handle);
}

bool Entity::operator==(const Entity& rhs) const
{
	return m_handle == rhs.m_handle && m_world == rhs.m_world;
}

bool Entity::operator!=(const Entity& rhs) const
{
	return m_handle != rhs.m_handle || m_world != rhs.m_world;
}

};


