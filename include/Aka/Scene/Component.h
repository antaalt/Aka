#pragma once

#include <Aka/Core/Geometry.h>
#include <Aka/Core/Sprite.h>
#include <Aka/Scene/Camera.h>
#include <Aka/Scene/Entity.h>

namespace aka {

enum class ComponentID : uint8_t {};

struct ComponentType
{
	// Get the id for the specified type
	template <typename T>
	static uint8_t get() {
		ASSERT(m_typeCounter < 255, "Reach max type capacity");
		static const uint8_t type = m_typeCounter++;
		return type;
	}
	// Current number of different type
	static uint8_t count() {
		return m_typeCounter;
	}
	// Maximum number of different type
	static constexpr uint8_t size() { return 255; }
private:
	static uint8_t m_typeCounter;
};

// Component
/*struct Transform2DComponent
{
	vec2f position;
	radianf rotation;
	vec2f scale;

	mat3f getTransform() const;
};

struct Transform3DComponent
{
	vec3f position;
	vec3f rotation;
	vec3f scale;

	mat4f getTransform() const;
};

struct CameraComponent
{
	Camera camera;
	bool primary;
};

struct RigidBody2DComponent
{
	static const vec2f maxVelocity;

	vec2f acceleration; // m/s^2
	vec2f velocity; // m/s
	float mass; // kg
};

enum class CollisionType
{
	Solid, // normal standard collision
	Event, // event collision 
};

struct CollisionEvent
{
	Entity dynamicEntity; // object who created the event
	Entity staticEntity; // object percuted
	CollisionType type;
};

struct Collider2DComponent
{
	CollisionType type;
	vec2f position;
	vec2f size;
	bool is(CollisionType);
};

struct SpriteAnimatorComponent
{
	Sprite* sprite;
};

// Render component
struct SpriteRenderComponent
{
};

struct Atlas
{
};

struct TileLayerRenderComponent
{
	Atlas atlas;
};

struct TextRenderComponent
{
};
// Audio
struct AudioComponent // Big sound like background music. To be streamed.
{
};

struct AudioFXComponent // Small fx entirely loaded to memory for responsivity.
{
};*/

};