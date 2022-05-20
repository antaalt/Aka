#pragma once

#include <Aka/Core/Geometry.h>
#include <Aka/Scene/Camera.h>
#include <Aka/Scene/Entity.h>

#include <string_view>

namespace aka {

enum class ComponentID : uint8_t {};

struct ComponentType
{
	// Get the id for the specified type
	template <typename T>
	static ComponentID get() {
		AKA_ASSERT(m_typeCounter < 255, "Too many components.");
		static const ComponentID type = static_cast<ComponentID>(m_typeCounter++);
		return type;
	}
	// Current number of different type
	static std::underlying_type<ComponentID>::type count() {
		return m_typeCounter;
	}
	// Maximum number of different type
	static constexpr std::underlying_type<ComponentID>::type size() { return 255; }
private:
	static std::underlying_type<ComponentID>::type m_typeCounter;
};

template <typename T>
struct ComponentHandle
{
	static const ComponentID id;
};
template <typename T>
const ComponentID ComponentHandle<T>::id = ComponentType::get<T>();

struct TagComponent
{
	String name;
};

// TODO in cpp file to auto instantiate
//template class ComponentHandle<Transform2D>;

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

static const vec2f maxVelocity;
struct RigidBody2DComponent
{
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
	uint32_t currentAnimation;
	uint32_t currentFrame;
};

// Render component
struct SpriteRenderComponent
{
	vec2f position;
	vec2f size;
	Texture::Ptr texture;
};

struct Atlas
{
	Texture::Ptr texture;
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