#pragma once

#include "Input.h"

namespace aka {

class InputBackend
{
public:
	static void initialize();
	static void destroy();
	static void update();
};

}