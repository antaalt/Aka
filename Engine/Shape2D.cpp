#include "Shape2D.h"

namespace app {

Shape2D::Shape2D(Type type) :
	m_type(type)
{
}

Collision Shape2D::overlaps(const Shape2D& shape) const
{
	switch (m_type)
	{
	case Shape2D::Type::Rect:
		switch (shape.m_type)
		{
			//case Shape2D::Type::Circle:
			//	return overlap(reinterpret_cast<const Rect&>(*this), reinterpret_cast<const Circle&>(shape));
		case Shape2D::Type::Rect:
			return overlap(reinterpret_cast<const Rect&>(*this), reinterpret_cast<const Rect&>(shape));
		default:
			return Collision::none();
		}
	case Shape2D::Type::Circle:
		switch (shape.m_type)
		{
			//case Shape2D::Type::Circle:
			//	return overlap(reinterpret_cast<const Rect&>(*this), reinterpret_cast<const Circle&>(shape));
		case Shape2D::Type::Rect:
			return overlap(reinterpret_cast<const Rect&>(*this), reinterpret_cast<const Rect&>(shape));
		default:
			return Collision::none();
		}
	default:
		return Collision::none();
	}
}

/*Collision overlap(const Circle& c1, const Circle& c2)
{
	return Collision::none();
}*/

Collision overlap(const Rect& r1, const Rect& r2)
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

/*Collision overlap(const Circle& c, const Rect& r)
{
	return Collision::none();
}

Collision overlap(const Rect& r, const Circle& c)
{
	return Collision::none();
}*/



Rect::Rect() :
	Shape2D(Type::Rect)
{
}

vec2f Rect::getNearestPoint(const vec2f& point) const
{
	vec2f nearest = point;
	nearest.x = max(nearest.x, this->position.x);
	nearest.x = min(nearest.x, this->position.x + this->size.x);
	nearest.y = max(nearest.y, this->position.y);
	nearest.y = min(nearest.y, this->position.y + this->size.y);
	return nearest;
}
bool Rect::isInside(const vec2f& point) const
{
	if (point.x < position.x || point.x > position.x + size.x)
		return false;
	if (point.y < position.y || point.y > position.y + size.y)
		return false;
	return true;
}

vec2f Rect::getPosition() const
{
	return position;
}
vec2f Rect::getSize() const
{
	return size;
}

void Rect::setPosition(const vec2f& position)
{
	this->position = position;
}


}
