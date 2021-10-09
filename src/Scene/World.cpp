#include <Aka/Scene/World.h>

#include <Aka/OS/OS.h>
#include <Aka/OS/Logger.h>
#include <Aka/Scene/Entity.h>
#include <Aka/Scene/Serializer.h>

namespace aka {

World::World()
{

}

World::~World()
{
	m_registry.clear();
	m_dispatcher.clear();
	for (std::unique_ptr<System>& system : m_systems)
		system->onDestroy(*this);
}

Entity World::createEntity(const String& name)
{
	Entity e = Entity(m_registry.create(), this);
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
	OS::File::write(path, str);
}

void World::load(const Path& path)
{
	String str;
	OS::File::read(path, &str);
	//*this = Parser::parse(str);
}

void World::onReceive(const AppUpdateEvent& event)
{
	for (std::unique_ptr<System>& system : m_systems)
		system->onUpdate(*this, event.deltaTime);
	m_dispatcher.update();
}

void World::onReceive(const AppFixedUpdateEvent& event)
{
	for (std::unique_ptr<System>& system : m_systems)
		system->onFixedUpdate(*this, event.deltaTime);
}

void World::onReceive(const AppRenderEvent& event)
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