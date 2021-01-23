#include "World.h"

#include "../../Platform/Logger.h"

namespace aka {

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

void World::each(std::function<void(Entity* entity)> callback)
{
	for (Entity* entity : m_entities)
		callback(entity);
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
	for (uint8_t i = 0; i < Event::Type::count(); i++)
	{
		for (Event* event : m_events[i])
			delete event;
		m_events[i].clear();
	}
}

void World::update(Time::Unit deltaTime)
{
	for (System* system : m_systems)
	{
		system->update(deltaTime);
	}
	// Destroy entities marked as dead
	for (auto it = m_entities.begin(); it != m_entities.end();)
	{
		if (!(*it)->m_alive)
		{
			// Destroy entity components before destroying entity
			for (Component* component : (*it)->m_components)
				destroyComponent(component);
			delete (*it);
			// Destroy entity
			it = m_entities.erase(it);
		}
		else
		{
			it++;
		}
	}
	// Clear all events
	for (uint8_t i = 0; i < Event::Type::count(); i++)
	{
		for (Event* event : m_events[i])
			delete event;
		m_events[i].clear();
	}
}

void World::draw(Batch& batch)
{
	for (System* system : m_systems)
		system->draw(batch);
}

};