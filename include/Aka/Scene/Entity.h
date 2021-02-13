#pragma once

#include <Aka/Scene/World.h>
#include <Aka/Scene/Component.h>

namespace aka {

class World;

class Entity
{
public:
	Entity();
	Entity(entt::entity handle, World* world);
	Entity(const Entity& entity) = default;
	Entity& operator=(const Entity& entity) = default;

	static Entity null();

	template <typename T, typename... Args>
	void add(Args&&... args);

	template <typename T>
	T& get() const;

	template <typename T>
	bool has() const;

	template <typename T>
	void remove();

	void destroy();

	bool valid();

	bool operator==(const Entity& rhs) const;
	bool operator!=(const Entity& rhs) const;

	entt::entity handle() const { return m_handle; }
	const World* world() const { return m_world; }
private:
	entt::entity m_handle;
	World* m_world;
};

template <typename T, typename... Args>
inline void Entity::add(Args&&... args)
{
	ComponentID idToRegister = ComponentType::get<T>();
	// TODO register this id ?
	m_world->registry().emplace<T>(m_handle, std::forward<Args>(args)...);
}

template <typename T>
inline T& Entity::get() const
{
	return m_world->registry().get<T>(m_handle);
}

template <typename T>
inline bool Entity::has() const
{
	return m_world->registry().has<T>(m_handle);
}

template <typename T>
inline void Entity::remove()
{
	return m_world->registry().remove<T>(m_handle);
}

}