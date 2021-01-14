#pragma once

#include "System.h"

namespace app {

class CameraSystem : public System
{
public:
	CameraSystem(World* world);

	virtual void update();
};

};
