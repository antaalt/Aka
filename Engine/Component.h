#pragma once

#include "GraphicBackend.h"

namespace app {

class Component
{
public:
	virtual void create(GraphicBackend& backend) = 0;
	virtual void destroy(GraphicBackend& backend) = 0;

	virtual void update() = 0;
	virtual void render(GraphicBackend& backend) = 0;
};

};