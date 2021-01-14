#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"
#include "GraphicBackend.h"

namespace app {

class World
{
public:
	World();

	Entity* createEntity();

	template <typename T, typename... Args>
	T* createSystem(Args&& ...args);

	void destroyEntity(Entity* entity);
	void destroyComponent(Component* component);
	void destroySystem(System* system);

	template <typename T>
	T *add(Entity& entity, T&& component);

	// Create all systems
	void create();
	// Destroy everything in this world (entities, components & systems)
	void destroy();
	// Update all systems
	void update();
	// Render all systems
	void render(GraphicBackend &backend);

public:
	// Iterator to loop over the world
	std::vector<Entity*>::iterator begin();
	std::vector<Entity*>::iterator end();
	std::vector<Entity*>::const_iterator begin() const;
	std::vector<Entity*>::const_iterator end() const;
private:
	// Using std list might be faster memory wise
	std::vector<Entity*> m_entities;
	std::vector<Component*> m_components[Component::Type::size()];
	std::vector<System*> m_systems;
};

template<typename T>
inline T* Entity::add(T&& component)
{
	static_assert(std::is_base_of<Component, T>::value, "Type is not a component");
	return m_world->add(*this, std::forward<T>(component));
}

template<typename T>
inline void Entity::remove()
{
	uint8_t type = Component::Type::get<T>();

	for (Component* component : m_components)
	{
		if (component->getType() == type)
		{
			m_world->destroyComponent(component);
			m_components.erase(component);
			break;
		}
	}
}

template <typename T>
T* World::add(Entity& entity, T&& component)
{
	static_assert(std::is_base_of<Component, T>::value, "Type is not a component");
	T* instance = new T;
	*instance = component;
	instance->m_type = Component::Type::get<T>();
	ASSERT(Component::Type::size() != Component::Type::count(), "Reached max component capacity");
	m_components[instance->m_type].push_back(instance);
	entity.m_components.push_back(instance);
	for (System* system : m_systems)
		system->add(&entity);
	return instance;
}

template <typename T, typename... Args>
inline T* World::createSystem(Args&& ...args)
{
	static_assert(std::is_base_of<System, T>::value, "Type is not a system");
	T *system = new T(std::forward<Args>(args)...);
	m_systems.push_back(system);
	for (Entity* entity : m_entities)
		system->add(entity);
	return system;
}

}

