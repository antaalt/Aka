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
	PhysicSimulation(float timestep);

	void create(GraphicBackend& backend);
	void destroy(GraphicBackend& backend);

	// Start the simulation
	void start();
	// Pause the simulation
	void pause();

	// Update the simulation
	void update();
	// Render colliders for debug
	void render(GraphicBackend& backend);

	DynamicRectCollider2D *createDynamicRectCollider();
	StaticRectCollider2D* createStaticRectCollider();
private:
	std::vector<DynamicCollider2D*> m_dynamics;
	std::vector<StaticCollider2D*> m_statics;
	std::vector<Collider2D*> m_colliders;
	float m_timestep;
	Time::unit m_lastTick;
	bool m_running;
};

}