#pragma once

#include <vector>

#include "Collider2D.h"
#include "GraphicBackend.h"
#include "Time.h"
#include "SpriteAnimatorComponent.h"

namespace app {

// TODO use pixel intersection
// TODO use box2D as lib instead (https://github.com/erincatto/box2d)
// https://2dengine.com/?p=collisions
class PhysicSimulation
{
public:
	PhysicSimulation(Time::Unit timestep);

	void create(GraphicBackend& backend);
	void destroy(GraphicBackend& backend);

	// Start the simulation
	void start();
	// Pause the simulation
	void pause();

	// Update the simulation
	void update();
	// Render colliders for debug
	void render(const Camera2D& camera, GraphicBackend& backend);

	DynamicRectCollider2D *createDynamicRectCollider();
	StaticRectCollider2D* createStaticRectCollider();

	std::vector<DynamicCollider2D*> dynamics;
	std::vector<StaticCollider2D*> statics;
	std::vector<Collider2D*> colliders;
	Time::Unit timestep;
	Time::Unit lastTick;
	bool running;
	bool renderColliders;
};

}