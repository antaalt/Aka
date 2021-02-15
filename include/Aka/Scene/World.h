#pragma once

#include <Aka/Scene/System.h>
#include <Aka/OS/Time.h>
#include <Aka/Core/Debug.h>
#include <Aka/Graphic/GraphicBackend.h>

#include <entt.hpp>


namespace aka {

class Entity;

class World
{
	friend class Entity;
public:
	// Create an entity
	Entity createEntity(const std::string& name);
	// Destroy an entity
	void destroyEntity(Entity entity);

	// Attach a system to the world.
	template <typename T>
	void attach();

	// Save the world to a file
	void save(const Path& path);
	// Load the world from a file
	void load(const Path& path);

	// Emit an event
	template <typename T>
	void emit(T&&);

	// Loop through all entities
	template <typename Func>
	void each(Func func) const;

	// Create all systems
	void create();
	// Destroy everything in this world
	void destroy();
	// Update all systems
	void update(Time::Unit deltaTime);
	// Draw all systems
	void draw(Batch& batch);

	// Get entt dispatcher
	entt::dispatcher& dispatcher();
	// Get entt registry
	entt::registry& registry();
private:
	std::vector<std::unique_ptr<System>> m_systems;
	entt::dispatcher m_dispatcher;
	entt::registry m_registry;
};

template <typename T>
inline void World::attach()
{
	static_assert(std::is_base_of<System, T>::value, "Type is not a system");
	m_systems.push_back(std::make_unique<T>());
}

template <typename T>
inline void World::emit(T&& event)
{
	m_dispatcher.enqueue<T>(std::forward<T>(event));
}

template <typename Func>
inline void World::each(Func func) const
{
	m_registry.each([&](entt::entity entity) {
		func(Entity(entity, const_cast<World*>(this)));
	});
}

}

