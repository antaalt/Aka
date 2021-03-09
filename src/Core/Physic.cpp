#include <Aka/Core/Physic.h>

namespace aka {

vec2f Line2D::nearest(const vec2f& point) const
{
	float cosTheta = vec2f::dot(end - start, point - start);
	return (point - start) * cosTheta;
}

bool Line2D::inside(const vec2f& point) const
{
	return false; // Cannot be inside a line.
}

vec2f Rect2D::nearest(const vec2f& point) const
{
	throw std::runtime_error("Not implemented");
}

bool Rect2D::inside(const vec2f& point) const
{
	vec2f AP = point - A;
	vec2f AB = B - A;
	vec2f AD = D - A;
	float APdotAB = vec2f::dot(AP, AB);
	float ABdotAB = vec2f::dot(AP, AB);
	float APdotAD = vec2f::dot(AP, AD);
	float ADdotAD = vec2f::dot(AD, AD);
	return (0.f <= APdotAB && APdotAB <= ABdotAB) && (0.f <= APdotAD && APdotAD <= ADdotAD);
}

vec2f AABB2D::nearest(const vec2f& point) const
{
	vec2f nearest = point;
	nearest.x = geometry::max(nearest.x, this->min.x);
	nearest.x = geometry::min(nearest.x, this->max.x);
	nearest.y = geometry::max(nearest.y, this->min.y);
	nearest.y = geometry::min(nearest.y, this->max.y);
	return nearest;
}

bool AABB2D::inside(const vec2f& point) const
{
	if (point.x < min.x || point.x > max.x)
		return false;
	if (point.y < min.y || point.y > max.y)
		return false;
	return true;
}

vec2f Circle2D::nearest(const vec2f& point) const
{
	vec2f dist = point - pos;
	float d = vec2f::dot(dist, dist);
	if (d < r * r)
		return point;
	return pos + vec2f::normalize(dist) * r;
}

bool Circle2D::inside(const vec2f& point) const
{
	return (point - pos).norm() <= r;
}

// https://github.com/RandyGaul/cute_headers/blob/master/cute_c2.h
bool Collision2D::overlap(const Shape2D& s0, const Shape2D& s1)
{
	if (dynamic_cast<const Rect2D*>(&s0) != nullptr)
	{
		if (dynamic_cast<const Rect2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Rect2D&>(s0), reinterpret_cast<const Rect2D&>(s1));
		else if (dynamic_cast<const Circle2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Rect2D&>(s0), reinterpret_cast<const Circle2D&>(s1));
		else if (dynamic_cast<const AABB2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Rect2D&>(s0), reinterpret_cast<const AABB2D&>(s1));
		else if (dynamic_cast<const Line2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Rect2D&>(s0), reinterpret_cast<const Line2D&>(s1));
	}
	else if (dynamic_cast<const Circle2D*>(&s0) != nullptr)
	{
		if (dynamic_cast<const Rect2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Circle2D&>(s0), reinterpret_cast<const Rect2D&>(s1));
		else if (dynamic_cast<const Circle2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Circle2D&>(s0), reinterpret_cast<const Circle2D&>(s1));
		else if (dynamic_cast<const AABB2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Circle2D&>(s0), reinterpret_cast<const AABB2D&>(s1));
		else if (dynamic_cast<const Line2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Circle2D&>(s0), reinterpret_cast<const Line2D&>(s1));
	}
	else if (dynamic_cast<const AABB2D*>(&s0) != nullptr)
	{
		if (dynamic_cast<const Rect2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const AABB2D&>(s0), reinterpret_cast<const Rect2D&>(s1));
		else if (dynamic_cast<const Circle2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const AABB2D&>(s0), reinterpret_cast<const Circle2D&>(s1));
		else if (dynamic_cast<const AABB2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const AABB2D&>(s0), reinterpret_cast<const AABB2D&>(s1));
		else if (dynamic_cast<const Line2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const AABB2D&>(s0), reinterpret_cast<const Line2D&>(s1));
	}
	else if (dynamic_cast<const Line2D*>(&s0) != nullptr)
	{
		if (dynamic_cast<const Rect2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Line2D&>(s0), reinterpret_cast<const Rect2D&>(s1));
		else if (dynamic_cast<const Circle2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Line2D&>(s0), reinterpret_cast<const Circle2D&>(s1));
		else if (dynamic_cast<const AABB2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Line2D&>(s0), reinterpret_cast<const AABB2D&>(s1));
		else if (dynamic_cast<const Line2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Line2D&>(s0), reinterpret_cast<const Line2D&>(s1));
	}
	return false;
}

bool Collision2D::overlap(const Rect2D& r0, const Rect2D& r1)
{
	// TODO optimize this
	Collision2D cl;
	return overlap(r0, r1, &cl);
}

bool Collision2D::overlap(const AABB2D& a0, const AABB2D& a1)
{
	bool d0 = a1.max.x < a0.min.x;
	bool d1 = a0.max.x < a1.min.x;
	bool d2 = a1.max.y < a0.min.y;
	bool d3 = a0.max.y < a1.min.y;
	return !(d0 || d1 || d2 || d3);
}

bool Collision2D::overlap(const Circle2D& c0, const Circle2D& c1)
{
	vec2f dist = c1.pos - c0.pos;
	float d = vec2f::dot(dist, dist);
	float r = c0.r + c1.r;
	return (d < r * r);
}

bool Collision2D::overlap(const Circle2D& c, const Rect2D& r)
{
	Line2D AB, BC, CD, DA;
	AB.start = r.A; AB.end = r.B;
	BC.start = r.B; BC.end = r.C;
	CD.start = r.C; CD.end = r.D;
	DA.start = r.D; DA.end = r.A;
	return r.inside(c.pos) || overlap(AB, c) || overlap(BC, c) || overlap(CD, c) || overlap(DA, c);
}

bool Collision2D::overlap(const Rect2D& r, const Circle2D& c)
{
	return overlap(c, r);
}

bool Collision2D::overlap(const AABB2D& a, const Rect2D& r)
{
	Collision2D cl;
	return overlap(a, r, &cl);
}

bool Collision2D::overlap(const Rect2D& r, const AABB2D& a)
{
	return overlap(a, r);
}

bool Collision2D::overlap(const Circle2D& c, const AABB2D& a)
{
	Line2D AB, BC, CD, DA;
	AB.start = a.min; AB.end = vec2f(a.min.x, a.max.y);
	BC.start = vec2f(a.min.x, a.max.y); BC.end = a.max;
	CD.start = a.max; CD.end = vec2f(a.max.x, a.min.y);
	DA.start = vec2f(a.max.x, a.min.y); DA.end = a.min;
	return a.inside(c.pos) || overlap(AB, c) || overlap(BC, c) || overlap(CD, c) || overlap(DA, c);
}

bool Collision2D::overlap(const AABB2D& a, const Circle2D& c)
{
	return overlap(c, a);
}

bool Collision2D::overlap(const Line2D& line, const Circle2D& circle)
{
	// https://math.stackexchange.com/questions/275529/check-if-line-intersects-with-circles-perimeter
	vec2f start = line.start - circle.pos;
	vec2f end = line.end - circle.pos;
	float a = (end.x - start.x) * (end.x - start.x) + (end.y - start.y) * (end.y - start.y);
	float b = 2 * (start.x * (end.x - start.x) + start.y * (end.y - start.y));
	float c = start.x * start.x + start.y * start.y - circle.r * circle.r;
	float disc = b * b - 4 * a * c;
	if (disc <= 0) 
		return false;
	float sqrtdisc = sqrt(disc);
	float t1 = (-b + sqrtdisc) / (2 * a);
	float t2 = (-b - sqrtdisc) / (2 * a);
	if ((0 < t1 && t1 < 1) || (0 < t2 && t2 < 1)) // TODO compute contact point
		return true;
	return false;
}

bool Collision2D::overlap(const Circle2D& c, const Line2D& l)
{
	return overlap(l, c);
}

bool Collision2D::overlap(const Shape2D& s0, const Shape2D& s1, Collision2D* collision)
{
	if (dynamic_cast<const Rect2D*>(&s0) != nullptr)
	{
		if (dynamic_cast<const Rect2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Rect2D&>(s0), reinterpret_cast<const Rect2D&>(s1), collision);
		else if (dynamic_cast<const Circle2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Rect2D&>(s0), reinterpret_cast<const Circle2D&>(s1), collision);
		else if (dynamic_cast<const AABB2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Rect2D&>(s0), reinterpret_cast<const AABB2D&>(s1), collision);
		else if (dynamic_cast<const Line2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Rect2D&>(s0), reinterpret_cast<const Line2D&>(s1), collision);
	}
	else if (dynamic_cast<const Circle2D*>(&s0) != nullptr)
	{
		if (dynamic_cast<const Rect2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Circle2D&>(s0), reinterpret_cast<const Rect2D&>(s1), collision);
		else if (dynamic_cast<const Circle2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Circle2D&>(s0), reinterpret_cast<const Circle2D&>(s1), collision);
		else if (dynamic_cast<const AABB2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Circle2D&>(s0), reinterpret_cast<const AABB2D&>(s1), collision);
		else if (dynamic_cast<const Line2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Circle2D&>(s0), reinterpret_cast<const Line2D&>(s1), collision);
	}
	else if (dynamic_cast<const AABB2D*>(&s0) != nullptr)
	{
		if (dynamic_cast<const Rect2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const AABB2D&>(s0), reinterpret_cast<const Rect2D&>(s1), collision);
		else if (dynamic_cast<const Circle2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const AABB2D&>(s0), reinterpret_cast<const Circle2D&>(s1), collision);
		else if (dynamic_cast<const AABB2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const AABB2D&>(s0), reinterpret_cast<const AABB2D&>(s1), collision);
		else if (dynamic_cast<const Line2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const AABB2D&>(s0), reinterpret_cast<const Line2D&>(s1), collision);
	}
	else if (dynamic_cast<const Line2D*>(&s0) != nullptr)
	{
		if (dynamic_cast<const Rect2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Line2D&>(s0), reinterpret_cast<const Rect2D&>(s1), collision);
		else if (dynamic_cast<const Circle2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Line2D&>(s0), reinterpret_cast<const Circle2D&>(s1), collision);
		else if (dynamic_cast<const AABB2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Line2D&>(s0), reinterpret_cast<const AABB2D&>(s1), collision);
		else if (dynamic_cast<const Line2D*>(&s1) != nullptr)
			return overlap(reinterpret_cast<const Line2D&>(s0), reinterpret_cast<const Line2D&>(s1), collision);
	}
	return false;
}

bool Collision2D::overlap(const Rect2D& r0, const Rect2D& r1, Collision2D* collision)
{
	throw std::runtime_error("Not implemented");
}

bool Collision2D::overlap(const AABB2D& a1, const AABB2D& a2, Collision2D* collision)
{
	// distance between the rects
	vec2f r1Pos = (a1.min + a1.max) / 2.f;
	vec2f r2Pos = (a2.min + a2.max) / 2.f;
	vec2f d = r1Pos - r2Pos;
	vec2f ad = vec2f::abs(d);
	// sum of the extents
	vec2f sh = (a1.max - a1.min) / 2.f + (a2.max - a2.min) / 2.f;
	if (ad.x >= sh.x || ad.y >= sh.y) // no intersections
		return false;
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
	collision->separation = s;
	return true;
}

bool Collision2D::overlap(const Circle2D& c0, const Circle2D& c1, Collision2D* collision)
{
	vec2f dist = c1.pos - c0.pos;
	float d = vec2f::dot(dist, dist);
	float r = c0.r + c1.r;
	if (d < r * r)
	{
		float length = sqrt(d);
		vec2f s = length != 0.f ? dist / length : vec2f(0.f, 1.f);
		collision->separation = s;
		return true;
	}
	else
		return false;
}

bool Collision2D::overlap(const Circle2D& c, const Rect2D& r, Collision2D* collision)
{
	// TODO compute separation
	Line2D AB, BC, CD, DA;
	AB.start = r.A; AB.end = r.B;
	BC.start = r.B; BC.end = r.C;
	CD.start = r.C; CD.end = r.D;
	DA.start = r.D; DA.end = r.A;
	if (r.inside(c.pos) || overlap(AB, c) || overlap(BC, c) || overlap(CD, c) || overlap(DA, c))
	{
		throw std::runtime_error("Not implemented");
		collision->separation = vec2f();
		return true;
	}
	return false;
}

bool Collision2D::overlap(const Rect2D& r, const Circle2D& c, Collision2D* collision)
{
	bool hit = overlap(c, r, collision);
	collision->separation = -collision->separation;
	return hit;
}

bool Collision2D::overlap(const AABB2D& a, const Rect2D& r, Collision2D* collision)
{
	throw std::runtime_error("Not implemented");
}

bool Collision2D::overlap(const Rect2D& r, const AABB2D& a, Collision2D* collision)
{
	bool hit = overlap(a, r, collision);
	collision->separation = -collision->separation;
	return hit;
}

bool Collision2D::overlap(const Circle2D& c, const AABB2D& a, Collision2D* collision)
{
	Line2D AB, BC, CD, DA;
	AB.start = a.min; AB.end = vec2f(a.min.x, a.max.y);
	BC.start = vec2f(a.min.x, a.max.y); BC.end = a.max;
	CD.start = a.max; CD.end = vec2f(a.max.x, a.min.y);
	DA.start = vec2f(a.max.x, a.min.y); DA.end = a.min;
	const bool inside = a.inside(c.pos);
	if (inside || overlap(AB, c) || overlap(BC, c) || overlap(CD, c) || overlap(DA, c))
	{
		collision->separation = vec2f();
		return true;
	}
	return false;
}

bool Collision2D::overlap(const AABB2D& a, const Circle2D& c, Collision2D* collision)
{
	bool hit = overlap(c, a, collision);
	collision->separation = -collision->separation;
	return hit;
}

};