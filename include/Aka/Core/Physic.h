#pragma once

#include <Aka/Core/Geometry.h>

namespace aka {

// https://github.com/RandyGaul/cute_headers/blob/master/cute_c2.h
// Box2D

struct Shape2D {
	// Get nearest point from shape
	virtual vec2f nearest(const vec2f& point) const = 0;
	// Check if point inside shape
	virtual bool inside(const vec2f& point) const = 0;
};

struct Line2D : Shape2D {
	vec2f start, end;

	vec2f nearest(const vec2f& point) const override;
	bool inside(const vec2f& point) const override;
};

struct Rect2D : Shape2D {
	vec2f A, B, C, D;

	vec2f nearest(const vec2f& point) const override;
	bool inside(const vec2f& point) const override;
};

struct AABB2D : Shape2D {
	vec2f min;
	vec2f max;

	vec2f nearest(const vec2f& point) const override;
	bool inside(const vec2f& point) const override;
};

struct Circle2D : Shape2D {
	vec2f pos;
	float r;

	vec2f nearest(const vec2f& point) const override;
	bool inside(const vec2f& point) const override;
};

struct Collision2D
{
	vec2f separation;

	static bool overlap(const Shape2D& s0, const Shape2D& s1);

	static bool overlap(const Rect2D& r0, const Rect2D& r1);
	static bool overlap(const AABB2D& a0, const AABB2D& a1);
	static bool overlap(const Circle2D& c0, const Circle2D& c1);

	static bool overlap(const Circle2D& c, const Rect2D& r);
	static bool overlap(const Rect2D& r, const Circle2D& c);
	static bool overlap(const AABB2D& a, const Rect2D& r);
	static bool overlap(const Rect2D& r, const AABB2D& a);
	static bool overlap(const Circle2D& c, const AABB2D& a);
	static bool overlap(const AABB2D& a, const Circle2D& c);
	static bool overlap(const Line2D& l, const Circle2D& c);
	static bool overlap(const Circle2D& c, const Line2D& l);


	static bool overlap(const Shape2D& s0, const Shape2D& s1, Collision2D* collision);

	static bool overlap(const Rect2D& r0, const Rect2D& r1, Collision2D* collision);
	static bool overlap(const AABB2D& a0, const AABB2D& a1, Collision2D* collision);
	static bool overlap(const Circle2D& c0, const Circle2D& c1, Collision2D* collision);

	static bool overlap(const Circle2D& c, const Rect2D& r, Collision2D* collision);
	static bool overlap(const Rect2D& r, const Circle2D& c, Collision2D* collision);
	static bool overlap(const AABB2D& a, const Rect2D& r, Collision2D* collision);
	static bool overlap(const Rect2D& r, const AABB2D& a, Collision2D* collision);
	static bool overlap(const Circle2D& c, const AABB2D& a, Collision2D* collision);
	static bool overlap(const AABB2D& a, const Circle2D& c, Collision2D* collision);
};



};