#pragma once

#include "GraphicBackend.h"
#include "Camera2D.h"

namespace app {

class Component
{
public:
	virtual void create(GraphicBackend& backend) = 0;
	virtual void destroy(GraphicBackend& backend) = 0;

	virtual void update() = 0;
	virtual void render(const Camera2D &camera, GraphicBackend& backend) = 0;
};

};