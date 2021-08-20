#pragma once

#include <Aka/Scene/System.h>
#include <Aka/OS/Time.h>
#include <Aka/Core/Debug.h>
#include <Aka/Graphic/GraphicBackend.h>

#include <entt.hpp>


namespace aka {

class Entity;
class World;

template <typename T>
class WorldEventListener
{
public:
	WorldEventListener(World& world);
	virtual ~WorldEventListener();
	virtual void receive(const T& event) = 0;
private:
	World& m_world;
};

class World
{
	template <typename T>
	friend class WorldEventListener;
	friend class Entity;
public:
	// Create an entity
	Entity createEntity(const String& name);
	// Destroy an entity
	void destroyEntity(Entity entity);

	// Attach a system to the world.
	template <typename T, typename... Args>
	void attach(Args&&... args);

	// Save the world to a file
	void save(const Path& path);
	// Load the world from a file
	void load(const Path& path);

	// Emit an event
	template <typename T>
	void emit(T&&);
	// Dispatch events of type T
	template <typename T>
	void dispatch();
	// Dispatch all events
	void dispatch();

	// Loop through all entities
	template <typename Func>
	void each(Func func) const;

	// Create all systems
	void create();
	// Destroy everything in this world
	void destroy();
	// Update all systems
	void update(Time::Unit deltaTime);
	// Update all systems
	void fixedUpdate(Time::Unit deltaTime);
	// Render all systems
	void render();

	// Get entt registry
	entt::registry& registry();
private:
	std::vector<std::unique_ptr<System>> m_systems;
	entt::dispatcher m_dispatcher;
	entt::registry m_registry;
};

template <typename T>
WorldEventListener<T>::WorldEventListener(World& world) :
	m_world(world)
{
	m_world.m_dispatcher.sink<T>().template connect<&WorldEventListener::receive>(*this);
}

template <typename T>
WorldEventListener<T>::~WorldEventListener()
{
	m_world.m_dispatcher.sink<T>().template disconnect<&WorldEventListener::receive>(*this);
}

template <typename T, typename... Args>
inline void World::attach(Args&&... args)
{
	static_assert(std::is_base_of<System, T>::value, "Type is not a system");
	m_systems.push_back(std::make_unique<T>(std::forward<Args>(args)...));
}

template <typename T>
inline void World::emit(T&& event)
{
	m_dispatcher.enqueue<T>(std::forward<T>(event));
}

template <typename T>
inline void World::dispatch()
{
	m_dispatcher.update<T>();
}

template <typename Func>
inline void World::each(Func func) const
{
	m_registry.each([&](entt::entity entity) {
		func(Entity(entity, const_cast<World*>(this)));
	});
}

}

