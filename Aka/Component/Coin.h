#pragma once

#include "../Core/ECS/Component.h"

namespace aka {

struct Coin : public Component {
	bool picked = false;
};


}