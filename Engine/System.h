#pragma once

#include "GraphicBackend.h"
#include "Batch.h"
#include "Component.h"
#include "Entity.h"
#include "Time.h"

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

	virtual void update(Time::Unit deltaTime);
	virtual void draw(Batch &batch);
protected:
	World *m_world;
};

}