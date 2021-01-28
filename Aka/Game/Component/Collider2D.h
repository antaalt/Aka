#pragma once

//#include "Shape2D.h"
#include "../../Core/Geometry.h"
#include "../../Core/ECS/Component.h"

namespace aka {

struct Collision2D
{
	bool collided;
	vec2f separation;
	static Collision2D none() { return Collision2D{ false, vec2f(0) }; }
	static Collision2D hit(vec2f separation) { return Collision2D{ true, separation }; }
};

/*struct Collider2D : public Component
{
	enum class Type {
		Rect,
		Circle
	};
	Collider2D(Type type);
	Collision2D overlaps(const Collider2D& collider);
	Type getType() const;
private:
	Type m_type;
};

struct RectCollider2D : public Collider2D
{
	vec2f position;
	vec2f size;
};

struct CircleCollider2D : public Collider2D
{
	vec2f position;
	float radius;
};*/

Collision2D overlap(const vec2f& p0, const vec2f &s0, const vec2f& p1, const vec2f& s1);


struct Collider2D : public Component
{
	Collider2D();
	Collider2D(const vec2f& position, const vec2f& size, float bouncing = 0.f, float friction = 0.f);

	//Collision2D overlaps(const Collider2D& collider);

	vec2f position;
	vec2f size;

	float bouncing;
	float friction;
};

struct RigidBody2D : public Component
{
	static inline const vec2f maxVelocity = vec2f(50.f);

	RigidBody2D();
	RigidBody2D(float mass);

	vec2f acceleration; // m/s^2
	vec2f velocity; // m/s
	float mass; // kg
};

/*struct Collider2D
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
};*/

}