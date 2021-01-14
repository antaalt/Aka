#pragma once

#include "GraphicBackend.h"
#include "Component.h"
#include "Entity.h"

#include <vector>
#include <set>

namespace aka {

class World;

// Physic
// IA
// Render
// Son

// Which component do I care about ?
class System
{
public:
	friend class World;

	System(World* world);

	virtual void create();
	virtual void destroy();

	virtual void update();
	virtual void render(GraphicBackend &backend);
protected:
	World *m_world;
};

}