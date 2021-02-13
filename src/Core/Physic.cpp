#include <Aka/Core/Physic.h>

namespace aka {

Collision2D overlap(const Rect2D& r1, const Rect2D& r2)
{
	// distance between the rects
	vec2f r1Pos = r1.pos + r1.size / 2.f;
	vec2f r2Pos = r2.pos + r2.size / 2.f;
	vec2f d = r1Pos - r2Pos;
	vec2f ad = vec2f::abs(d);
	// sum of the extents
	vec2f sh = r1.size / 2.f + r2.size / 2.f;
	if (ad.x >= sh.x || ad.y >= sh.y) // no intersections
		return Collision2D::none();
	// shortest separation
	vec2f s = sh - ad;
	// ignore longer axis
	if (s.x < s.y)
	{
		if (s.x > 0.f)
			s.y = 0.f;
	}
	else
	{
		if (s.y > 0.f)
			s.x = 0.f;
	}
	// correct sign
	if (d.x < 0.f)
		s.x = -s.x;
	if (d.y < 0.f)
		s.y = -s.y;
	return Collision2D::hit(s);
}

};