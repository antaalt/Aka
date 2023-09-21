#pragma once

#include <Aka/Scene/System.h>
#include <Aka/OS/Time.h>
#include <Aka/Core/Config.h>
#include <Aka/Core/Application.h>
#include <Aka/Scene/Entity.h>

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
	World();
	~World();

	// Create an entity
	Entity createEntity(const String& name);
	// Destroy an entity
	void destroyEntity(Entity entity);
	// Destroy all entities from the world
	void clear();

	// Attach a system to the world.
	template <typename T, typename... Args> void attach(Args&&... args);
	// Detach a system from the world.
	template <typename T> void detach();
	// Get a system from the world
	template <typename T> T& get();

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

	// Update all systems
	void fixedUpdate(Time deltaTime);
	// Update all systems
	void update(Time deltaTime);
	// Render all systems
	void render(gfx::Frame* frame);
	// Resize all systems
	void resize(uint32_t width, uint32_t height);

	// Get entt registry
	entt::registry& registry();
	const entt::registry& registry() const;
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
	for (auto& system : m_systems)
		if (typeid(*system) == typeid(T))
			return; // already attached.
	std::unique_ptr<System> ptr = std::make_unique<T>(std::forward<Args>(args)...);
	ptr->onCreate(*this);
	m_systems.push_back(std::move(ptr));
}

template <typename T>
inline void World::detach()
{
	static_assert(std::is_base_of<System, T>::value, "Type is not a system");
	for (auto& it = m_systems.begin(); it != m_systems.end(); it++)
	{
		auto& system = *it;
		if (typeid(*system) == typeid(T))
		{
			system->onDestroy(*this);
			m_systems.erase(it);
			break;
		}
	}
}

template <typename T>
inline T& World::get()
{
	static_assert(std::is_base_of<System, T>::value, "Type is not a system");
	for (auto& system : m_systems)
		if (typeid(*system) == typeid(T))
			return reinterpret_cast<T&>(*system);
	throw std::runtime_error("System not attached");
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

