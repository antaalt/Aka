#include "System.h"

namespace app {

System::System(World* world) : 
	m_world(world)
{
}

/*void System::add(Entity* entity)
{
	if (valid(entity))
		m_entities.insert(entity);
}

void System::remove(Entity* entity)
{
	m_entities.erase(entity);
}*/

/*bool System::valid(Entity* entity)
{
	// By default, consider a system has no requirement
	return true;
}*/

void System::create() {}
void System::destroy() {}
void System::update() {}
void System::render(GraphicBackend& backend) {}

};