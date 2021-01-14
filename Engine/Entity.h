#pragma once

#include <vector>

#include "Component.h"

namespace app {

class World;

// Player
// Ennemy
struct Entity final
{
	friend class World;

	// Add a component to the entity and return it
	template <typename T>
	T* add(T&& component);

	template <typename T>
	void remove();

	// Get the component for the entity
	template <typename T>
	T* get();
	template <typename T>
	const T* get() const;

	// Check if the entity has certain components
	template <typename T>
	bool has() const;
	template <typename T, typename U, typename... Args>
	bool has() const;

private:
	Entity(World* world);
	~Entity() {}

	World* m_world;
	std::vector<Component*> m_components;
};


template<typename T>
inline T* Entity::get()
{
	// TODO use std set instead ? or std list
	static_assert(std::is_base_of<Component, T>::value, "Type is not a component");
	uint8_t type = Component::Type::get<T>();
	for (Component* component : m_components)
		if (component->getType() == type)
			return reinterpret_cast<T*>(component);
	return nullptr;
}

template<typename T>
inline const T* Entity::get() const
{
	static_assert(std::is_base_of<Component, T>::value, "Type is not a component");
	uint8_t type = Component::Type::get<T>();
	for (const Component* component : m_components)
		if (component->getType() == type)
			return reinterpret_cast<T*>(component);
	return nullptr;
}

template<typename T>
inline bool Entity::has() const
{
	static_assert(std::is_base_of<Component, T>::value, "Type is not a component");
	uint8_t type = Component::Type::get<T>();
	for (const Component* component : m_components)
		if (component->getType() == type)
			return true;
	return false;
}

template <typename T, typename U, typename... Args>
inline bool Entity::has() const
{
	return has<T>() && has<U, Args...>();
}

}