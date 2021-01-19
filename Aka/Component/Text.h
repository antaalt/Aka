#pragma once

#include "../Core/Font.h"
#include "../Core/Component.h"


namespace aka {

struct Text : public Component
{
	Font *font;
	std::string text;
	color4f color;
	int32_t layer;
};

}

