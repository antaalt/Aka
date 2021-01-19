#pragma once

#include "../Platform/Time.h"
#include "../Core/System.h"
#include "../Component/Collider2D.h"

namespace aka {

// TODO use pixel intersection
// TODO use box2D as lib instead (https://github.com/erincatto/box2d)
// https://2dengine.com/?p=collisions
class PhysicSystem : public System
{
public:
	PhysicSystem(World* world, Time::Unit timestep);

	void update(Time::Unit deltaTime) override;
private:
	Time::Unit m_accumulator;
	Time::Unit m_timestep;
};

};
