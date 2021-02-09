#include <Aka/Core/ECS/System.h>

namespace aka {

System::System(World* world) : 
	m_world(world)
{
}

void System::create() {}
void System::destroy() {}
void System::update(Time::Unit deltaTime) {}
void System::draw(Batch &batch) {}

};