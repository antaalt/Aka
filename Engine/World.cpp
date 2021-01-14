#include "World.h"

#include "Logger.h"

namespace app {

World::World()
{
}

Entity* World::createEntity()
{
	Entity* entity = new Entity(this);
	m_entities.push_back(entity);
	return entity;
}

void World::destroyEntity(Entity* entity)
{
	auto it = std::find(m_entities.begin(), m_entities.end(), entity);
	if (it == m_entities.end())
	{
		Logger::warn("Could not find entity to destroy : ", entity);
	}
	else
	{
		// Destroy entity components before destroying entity
		for (Component* component : entity->m_components)
			destroyComponent(component);
		// Destroy entity
		m_entities.erase(it);
		delete entity;
	}
}

void World::destroyComponent(Component* component)
{
	uint8_t type = component->getType();
	auto it = std::find(m_components[type].begin(), m_components[type].end(), component);
	if (it == m_components[type].end())
	{
		Logger::warn("Could not find component to destroy : ", component);
	}
	else
	{
		m_components[type].erase(it);
		delete component;
	}
}

void World::destroySystem(System* system)
{
	auto it = std::find(m_systems.begin(), m_systems.end(), system);
	if (it == m_systems.end())
	{
		Logger::warn("Could not find system to destroy : ", system);
	}
	else
	{
		m_systems.erase(it);
		delete system;
	}
}

void World::create()
{
	for (System* system : m_systems)
		system->create();
}

void World::destroy()
{
	for (System* system : m_systems)
	{
		system->destroy();
		delete system;
	}
	m_systems.clear();
	for (uint8_t i = 0; i < Component::Type::count(); i++)
	{
		for (Component* component : m_components[i])
			delete component;
		m_components[i].clear();
	}
	for (Entity* entity : m_entities)
		delete entity;
	m_entities.clear();
}

void World::update()
{
	for (System* system : m_systems)
	{
		system->update();
	}
}

void World::render(GraphicBackend& backend)
{
	for (System* system : m_systems)
		system->render(backend);
}

};