#pragma once

#include "Font.h"
#include "Component.h"


namespace aka {

struct Text : public Component
{
	Font *font;
	std::string text;
	color4f color;
};

}

