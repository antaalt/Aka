#pragma once

#include "Shape2D.h"

namespace app {

struct Collider2D
{
	Collider2D(Shape2D* shape);

	Collision overlaps(const Collider2D& collider);
	Shape2D* getShape();
	// is on floor function

	float bouncing;
	float friction;
private:
	Shape2D* m_shape;
};

struct StaticCollider2D : public Collider2D
{
	StaticCollider2D(Shape2D*shape);
	virtual ~StaticCollider2D() = 0;
};

struct DynamicCollider2D : public Collider2D
{
	DynamicCollider2D(Shape2D* shape);
	virtual ~DynamicCollider2D() = 0;

	// Add a force to the dynamic object
	void addForce(const vec2f& force, float dt);
	// Move the object depending on its force
	virtual void move(float dt) = 0;
	// Resolve the collision
	void resolve(const vec2f& separation, const Collider2D &other);

	static const vec2f maxVelocity; // velocity thresold
	
	float mass;
	vec2f velocity;
	vec2f acceleration;
};

struct StaticRectCollider2D : public StaticCollider2D
{
	StaticRectCollider2D();

	Rect rect;
};

struct DynamicRectCollider2D : public DynamicCollider2D
{
	DynamicRectCollider2D();

	void move(float dt) override;

	Rect rect;
};

}