#pragma once

#include "../../OS/Time.h"
#include "../../Core/ECS/System.h"
#include "../Component/Collider2D.h"

namespace aka {

// TODO use pixel intersection
// TODO use box2D as lib instead (https://github.com/erincatto/box2d)
// https://2dengine.com/?p=collisions
class PhysicSystem : public System
{
public:
	PhysicSystem(World* world);

	void update(Time::Unit deltaTime) override;
};

};
