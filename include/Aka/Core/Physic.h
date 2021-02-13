#pragma once

#include <Aka/Core/Geometry.h>

namespace aka {

struct Collision2D
{
	bool collided;
	vec2f separation;
	static Collision2D none() { return Collision2D{ false, vec2f(0) }; }
	static Collision2D hit(vec2f separation) { return Collision2D{ true, separation }; }
};

struct Rect2D {
	vec2f pos;
	vec2f size;
};

struct Circle2D {
	vec2f pos;
	float r;
};

Collision2D overlap(const Rect2D& r0, const Rect2D& r1);
//Collision2D overlap(const Circle2D& r0, const Circle2D& r1);
//Collision2D overlap(const Rect2D& r0, const Circle2D& r1);
//Collision2D overlap(const Circle2D& r0, const Rect2D& r1);

};