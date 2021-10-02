#pragma once

#include <Aka/Drawing/Renderer2D.h>
#include <Aka/Drawing/Renderer3D.h>
#include <Aka/OS/Time.h>

namespace aka {

class World;

class System
{
public:
	System() {}
	virtual ~System() {}
	friend class World;

protected:
	virtual void onCreate(World &world) {}
	virtual void onDestroy(World& world) {}

	virtual void onFixedUpdate(World& world, Time deltaTime) {}
	virtual void onUpdate(World& world, Time deltaTime) {}
	virtual void onRender(World& world) {}
};

}