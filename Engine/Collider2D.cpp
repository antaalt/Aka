#include "Collider2D.h"

namespace app {

// https://2dengine.com/?p=intersections
vec2f RectCollider2D::getNearestPoint(const vec2f& point) const
{
	vec2f nearest = point;
	nearest.x = max(nearest.x, this->position.x);
	nearest.x = min(nearest.x, this->position.x + this->size.x);
	nearest.y = max(nearest.y, this->position.y);
	nearest.y = min(nearest.y, this->position.y + this->size.y);
	return nearest;
}

bool RectCollider2D::isInside(const vec2f& point) const
{
	if (point.x < position.x || point.x > position.x + size.x)
		return false;
	if (point.y < position.y || point.y > position.y + size.y)
		return false;
	return true;
}

/*vec2f CircleCollider2D::getNearestPoint(const vec2f& point) const
{
	vec2f dxy = point - this->position;
	float d = dxy.norm();
	if (d <= radius)
		return point;
	return dxy / d * radius + position;
}

bool CircleCollider2D::isInside(const vec2f& point) const
{
	vec2f dxy = point - position;
	return vec2f::dot(dxy, dxy) <= radius * radius;
}
*/
/*vec2f TriangleCollider2D::getNearestPoint(const vec2f& point) const
{
	vec2f AB = B - A;
	vec2f AC = C - A;
	vec2f AP = point - A;
	// vertex region outside a
	float d1 = vec2f::dot(AB, AP);
	float d2 = vec2f::dot(AC, AP);
	if (d1 <= 0.f && d2 <= 0.f)
		return A;
	// vertex region outside b
	vec2f BP = point - B;
	float d3 = vec2f::dot(AB, BP);
	float d4 = vec2f::dot(AC, BP);
	if (d3 >= 0.f && d4 <= d3)
		return A;
	// edge region ab
	if (d1 >= 0 && d3 <= 0 && d1 * d4 - d3 * d2 <= 0)
	{
		float v = d1 / (d1 - d3);
		return A + AB * v;
	}
	// vertex region outside c
	vec2f CP = point - C;
	float d5 = vec2f::dot(AB, CP);
	float d6 = vec2f::dot(AC, CP);
	if (d6 >= 0 && d5 <= d6)
		return C;
	// edge region ac
	if (d2 >= 0 && d6 <= 0 && d5 * d2 - d1 * d6 <= 0)
	{
		float w = d2 / (d2 - d6);
		return A + AC * w;
	}
	// edge region bc
	if (d3*d6 - d5*d4 <= 0)
	{
		float d43 = d4 - d3;
		float d56 = d5 - d6;
		if (d43 >= 0 && d56 >= 0)
		{
			float w = d43 / (d43 + d56);
			return B + (C - B) * w;
		}
	}
	// inside face region
	return point;
}

bool TriangleCollider2D::isInside(const vec2f& point) const
{
	vec2f p1 = A - point;
	vec2f p2 = B - point;
	vec2f p3 = C - point;
	bool sab = p1.x * p2.y - p1.y * p2.x < 0;
	if (sab != (p2.x * p3.y - p2.y * p3.x < 0.f))
		return false;
	return sab == (p3.x * p1.y - p3.y * p1.x < 0);
}*/

/*Collision overlap(const CircleCollider2D& c1, const CircleCollider2D& c2)
{
	float radius = c1.radius + c2.radius;
	vec2f d = c2.position - c1.position;
	return vec2f::dot(d, d)<= radius * radius;
}*/

Collision overlap(const RectCollider2D& r1, const RectCollider2D& r2)
{
	// distance between the rects
	vec2f r1Pos = r1.position + r1.size / 2.f;
	vec2f r2Pos = r2.position + r2.size / 2.f;
	vec2f d = r1Pos - r2Pos;
	vec2f ad = vec2f::abs(d);
	// sum of the extents
	vec2f sh = r1.size / 2.f + r2.size / 2.f;
	if (ad.x >= sh.x || ad.y >= sh.y) // no intersections
		return Collision::none();
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
	return Collision::hit(s);
}

/*Collision overlap(const TriangleCollider2D& t1, const TriangleCollider2D& t2)
{
	throw std::runtime_error("Not implemented");
}*/

/*Collision overlap(const CircleCollider2D& c, const RectCollider2D& r)
{
	vec2f d = clamp(c.position, r.position, r.position + r.size) - c.position;
	return vec2f::dot(d, d) <= c.radius * c.radius;
}

Collision overlap(const RectCollider2D& r, const CircleCollider2D& c)
{
	return overlap(c, r);
}*/

/*Collision overlap(const TriangleCollider2D& t, const CircleCollider2D& c)
{
	throw std::runtime_error("Not implemented");
}

Collision overlap(const CircleCollider2D& c, const TriangleCollider2D& t)
{
	return overlap(t, c);
}

Collision overlap(const TriangleCollider2D& t, const RectCollider2D& r)
{
	throw std::runtime_error("Not implemented");
}

Collision overlap(const RectCollider2D& r, const TriangleCollider2D& t)
{
	return overlap(t, r);
}*/

}