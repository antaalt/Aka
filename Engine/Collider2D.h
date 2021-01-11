#pragma once

#include "Geometry.h"

namespace app {

struct Collider2D
{
	virtual vec2f getNearestPoint(const vec2f & point) const = 0;
	virtual bool isInside(const vec2f& point) const = 0;
	//virtual bool overlaps(const Collider2D& collider) const = 0;
};

struct RectCollider2D : public Collider2D
{
	vec2f getNearestPoint(const vec2f& point) const override;
	bool isInside(const vec2f& point) const override;

	vec2f position; //top left
	vec2f size;
};

/*struct CircleCollider2D : public Collider2D
{
	vec2f getNearestPoint(const vec2f& point) const override;
	bool isInside(const vec2f& point) const override;

	vec2f position;
	float radius;
};*/

/*struct TriangleCollider2D : public Collider2D
{
	vec2f getNearestPoint(const vec2f& point) const override;
	bool isInside(const vec2f& point) const override;

	vec2f A, B, C;
};*/

struct Collision {
	bool collided;
	vec2f separation;
	static Collision none() { return Collision{false, vec2f(0)}; }
	static Collision hit(vec2f separation) { return Collision{ true, separation }; }
};

//Collision overlap(const CircleCollider2D& c1, const CircleCollider2D& c2);
Collision overlap(const RectCollider2D& c1, const RectCollider2D& c2);
//Collision overlap(const TriangleCollider2D& c1, const TriangleCollider2D& c2);

/*Collision overlap(const CircleCollider2D& c1, const RectCollider2D& c2);
Collision overlap(const RectCollider2D& c1, const CircleCollider2D& c2);*/
/*Collision overlap(const TriangleCollider2D& c1, const CircleCollider2D& c2);
Collision overlap(const CircleCollider2D& c1, const TriangleCollider2D& c2);
Collision overlap(const TriangleCollider2D& c1, const RectCollider2D& c2);
Collision overlap(const RectCollider2D& c1, const TriangleCollider2D& c2);*/


}