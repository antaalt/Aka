#pragma once

#include <Aka/OS/Time.h>

namespace aka {

namespace gfx {
struct Frame;
};

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
	virtual void onRender(World& world, gfx::Frame* frame) {}

	virtual void onResize(World& world, uint32_t width, uint32_t height) {}
};

}