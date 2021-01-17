#include "System.h"

namespace aka {

System::System(World* world) : 
	m_world(world)
{
}

void System::create() {}
void System::destroy() {}
void System::update() {}
void System::draw(Batch &batch) {}

};