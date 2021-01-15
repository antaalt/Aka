#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"
#include "Time.h"
#include "Debug.h"
#include "GraphicBackend.h"

#include <functional>
#include <type_traits>


namespace aka {

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

	template <typename T>
	void each(std::function<void(Entity* entity, T*)> callback);
	template <typename T, typename U, typename... Args>
	void each(typename std::common_type<std::function<void(Entity*, T*, U*, Args*...)>>::type callback);

	// Create all systems
	void create();
	// Destroy everything in this world (entities, components & systems)
	void destroy();
	// Update all systems
	void update();
	// Render all systems
	void render(GraphicBackend &backend);
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
	for (auto it = m_components.begin(); it != m_components.end(); it++)
	{
		if ((*it)->getType() == type)
		{
			m_world->destroyComponent(*it);
			m_components.erase(it);
			break;
		}
	}
}

template <typename T>
T* World::add(Entity& entity, T&& component)
{
	static_assert(std::is_base_of<Component, T>::value, "Type is not a component");
	T* old = entity.get<T>();
	if (old != nullptr)
		return old;
	T* instance = new T;
	*instance = component;
	instance->m_type = Component::Type::get<T>();
	instance->m_entity = &entity;
	ASSERT(Component::Type::size() != Component::Type::count(), "Reached max component capacity");
	m_components[instance->m_type].push_back(instance);
	entity.m_components.push_back(instance);
	return instance;
}

template <typename T>
inline void World::each(std::function<void(Entity* entity, T*)> callback)
{
	uint8_t type = Component::Type::get<T>();
	for (Component* component : m_components[type])
		callback(component->m_entity, reinterpret_cast<T*>(component));
}

template <typename T, typename U, typename... Args>
inline void World::each(typename std::common_type<std::function<void(Entity* entity, T*, U*, Args*...)>>::type callback)
{
	uint8_t type = Component::Type::get<T>();
	for (Component* component : m_components[type])
		if (component->m_entity->has<U, Args...>())
			callback(component->m_entity, reinterpret_cast<T*>(component), reinterpret_cast<U*>(component->m_entity->get<U>()), component->m_entity->get<Args>()...);
}

template <typename T, typename... Args>
inline T* World::createSystem(Args&& ...args)
{
	static_assert(std::is_base_of<System, T>::value, "Type is not a system");
	T *system = new T(this, std::forward<Args>(args)...);
	m_systems.push_back(system);
	return system;
}

}

