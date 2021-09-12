#include <Aka/Scene/World.h>

#include <Aka/OS/Logger.h>
#include <Aka/Scene/Entity.h>
#include <Aka/Scene/Serializer.h>

namespace aka {

Entity World::createEntity(const String& name)
{
	Entity e =  Entity(m_registry.create(), this);
	e.add<TagComponent>();
	e.get<TagComponent>().name = name;
	return e;
}

void World::destroyEntity(Entity entity)
{
	m_registry.destroy(entity.handle());
}

void World::save(const Path& path)
{
	String str = Serializer::serialize(*this);
	File::write(path, str);
}

void World::load(const Path& path)
{
	String str;
	File::read(path, &str);
	//*this = Parser::parse(str);
}

void World::create()
{
	for (std::unique_ptr<System>& system : m_systems)
		system->onCreate(*this);
}

void World::destroy()
{
	m_dispatcher.clear();
	m_registry.clear();
	for (std::unique_ptr<System>& system : m_systems)
		system->onDestroy(*this);
	m_systems.clear();
}

void World::update(Time::Unit deltaTime)
{
	for (std::unique_ptr<System>& system : m_systems)
		system->onUpdate(*this, deltaTime);
	m_dispatcher.update();
}

void World::fixedUpdate(Time::Unit deltaTime)
{
	for (std::unique_ptr<System>& system : m_systems)
		system->onFixedUpdate(*this, deltaTime);
}

void World::render()
{
	for (std::unique_ptr<System>& system : m_systems)
		system->onRender(*this);
}

entt::registry& World::registry()
{
	return m_registry;
}

const entt::registry& World::registry() const
{
	return m_registry;
}

};