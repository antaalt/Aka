#pragma once

#include "Time.h"
#include "System.h"
#include "Collider2D.h"

namespace app {

// TODO use pixel intersection
// TODO use box2D as lib instead (https://github.com/erincatto/box2d)
// https://2dengine.com/?p=collisions
class PhysicSystem : public System
{
public:
	PhysicSystem(Time::Unit timestep);

	void update() override;
public:
	bool valid(Entity* entity) override;
private:
	Time::Unit m_timestep;
	std::vector<RigidBody2D> m_rigids;
	std::vector<Collider2D> m_colliders;
};

};
