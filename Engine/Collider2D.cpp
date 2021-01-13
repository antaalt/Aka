#include "Collider2D.h"

namespace app {


Collider2D::Collider2D(Shape2D* shape) :
	m_shape(shape)
{
}

Collision Collider2D::overlaps(const Collider2D& collider)
{
	return m_shape->overlaps(*collider.m_shape);
}
Shape2D* Collider2D::getShape()
{
	return m_shape;
}

StaticCollider2D::StaticCollider2D(Shape2D* shape) :
	Collider2D(shape)
{
}
StaticCollider2D::~StaticCollider2D()
{
}

const vec2f DynamicCollider2D::maxVelocity(5.f, 5.f);

DynamicCollider2D::DynamicCollider2D(Shape2D* shape) :
	Collider2D(shape),
	mass(1.f),
	velocity(0.f),
	acceleration(0.f)
{
}
DynamicCollider2D::~DynamicCollider2D()
{
}

void DynamicCollider2D::addForce(const vec2f& force, float dt)
{
	acceleration += (force / mass) * dt;
	velocity += acceleration * dt;
	velocity.x = clamp(velocity.x, -maxVelocity.x, maxVelocity.x);
	velocity.y = clamp(velocity.y, -maxVelocity.y, maxVelocity.y);
}

void DynamicCollider2D::resolve(const vec2f& separation, const Collider2D& other)
{
	// Adjust velocity
	// Get normal 
	vec2f normal = vec2f::normalize(separation);
	// Get relative velocity
	vec2f v = this->velocity; // substract by second object velocity if it has one
	// Get penetration speed
	float ps = vec2f::dot(v, normal);
	// objects moving towards each other ?
	if (ps <= 0.f)
	{
		// Move the rigid to avoid overlapping.
		getShape()->setPosition(getShape()->getPosition() + separation);
	}
	// Get penetration component
	vec2f p = normal * ps;
	// tangent component
	vec2f t = v - p;
	// Restitution
	float r = 1.f + max<float>(bouncing, other.bouncing); // max bouncing value of object a & b
	// Friction
	float f = min<float>(friction, other.friction); // max friction value of object a & b
	// Change the velocity of shape a
	this->acceleration = vec2f(0);
	this->velocity = this->velocity - p * r + t * f;
}

StaticRectCollider2D::StaticRectCollider2D() :
	StaticCollider2D(&rect)
{
}

DynamicRectCollider2D::DynamicRectCollider2D() :
	DynamicCollider2D(&rect)
{
}

void DynamicRectCollider2D::move(float dt)
{
	rect.position += velocity;// *dt;
}























/*CircleCollider2D::CircleCollider2D() :
	Collider2D(Collider2D::Type::Circle),
	radius(0.f)
{
}

vec2f CircleCollider2D::getNearestPoint(const vec2f& point) const
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
}*/

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
	//float radius = c1.radius + c2.radius;
	//vec2f d = c2.position - c1.position;
	//return vec2f::dot(d, d)<= radius * radius;
	return Collision::none();
}

/*Collision overlap(const CircleCollider2D& c, const RectCollider2D& r)
{
	//vec2f d = clamp(c.position, r.position, r.position + r.size) - c.position;
	//return vec2f::dot(d, d) <= c.radius * c.radius;
	return Collision::none();
}
*/

}