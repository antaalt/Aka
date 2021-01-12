#pragma once

#include "Geometry.h"

namespace app {

struct Collision
{
	bool collided;
	vec2f separation;
	static Collision none() { return Collision{ false, vec2f(0) }; }
	static Collision hit(vec2f separation) { return Collision{ true, separation }; }
};

struct Shape2D
{
	enum class Type {
		Rect,
		Circle
	};
	Shape2D(Type type);
	Collision overlaps(const Shape2D& shape) const;
	virtual vec2f getNearestPoint(const vec2f& point) const = 0;
	virtual bool isInside(const vec2f& point) const = 0;

	virtual vec2f getPosition() const = 0;
	virtual vec2f getSize() const = 0;

	virtual void setPosition(const vec2f& position) = 0;
private:
	const Type m_type;
};

struct Rect : public Shape2D
{
	Rect();
	vec2f getNearestPoint(const vec2f& point) const override;
	bool isInside(const vec2f& point) const override;

	vec2f getPosition() const override;
	vec2f getSize() const override;

	void setPosition(const vec2f& position) override;

	vec2f position;
	vec2f size;
};

/*struct Circle : public Shape2D
{
	Circle();
	vec2f getNearestPoint(const vec2f& point) const override;
	bool isInside(const vec2f& point) const override;

	vec2f getPosition() const override;
	vec2f getSize() const override;

	vec2f position;
	float radius;
};*/

//Collision overlap(const Circle& c1, const Circle& c2);
Collision overlap(const Rect& r1, const Rect& r2);
//Collision overlap(const Circle& c, const Rect& r);
//Collision overlap(const Rect& r, const Circle& c);

}

