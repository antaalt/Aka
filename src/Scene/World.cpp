#include <Aka/Scene/World.h>

#include <Aka/OS/Logger.h>
#include <Aka/Scene/Entity.h>

namespace aka {

Entity World::createEntity(const std::string& name)
{
	return Entity(m_registry.create(), this);
}

void World::destroyEntity(Entity entity)
{
	m_registry.destroy(entity.handle());
}

void World::destroySystem(System& system)
{
	auto it = std::find_if(m_systems.begin(), m_systems.end(), [&system](const std::unique_ptr<System> &s) {
		return s.get() == &system;
	});
	if (it == m_systems.end())
		Logger::warn("Could not find system to destroy");
	else
		m_systems.erase(it);
}

void World::save(const Path& path)
{

}

void World::load(const Path& path)
{

}

void World::create()
{
	for (std::unique_ptr<System>& system : m_systems)
		system->create(*this);
}

void World::destroy()
{
	for (std::unique_ptr<System>& system : m_systems)
		system->destroy(*this);
}

void World::update(Time::Unit deltaTime)
{
	for (std::unique_ptr<System>& system : m_systems)
		system->update(*this, deltaTime);
	m_dispatcher.update();
}

void World::draw(Batch& batch)
{
	for (std::unique_ptr<System>& system : m_systems)
		system->draw(*this, batch);
}

entt::dispatcher& World::dispatcher()
{
	return m_dispatcher;
}

entt::registry& World::registry()
{
	return m_registry;
}

};