#include "Text.h"

namespace aka {

Text::Text() :
	Text(vec2f(0.f), nullptr, "", color4f(1.f), 0)
{
}

Text::Text(const vec2f& offset, Font* font, const std::string& text, const color4f& color, int32_t layer) :
	offset(offset),
	font(font),
	text(text),
	color(color),
	layer(layer)
{
}

}
