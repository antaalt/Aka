#pragma once

#include <Aka/Graphic/Batch.h>
#include <Aka/OS/Time.h>

namespace aka {

class World;

class System
{
public:
	virtual ~System() = default;
	friend class World;

	virtual void create(World &world) {}
	virtual void destroy(World& world) {}

	virtual void update(World& world, Time::Unit deltaTime) {}
	virtual void draw(World& world, Batch &batch) {}
};

}