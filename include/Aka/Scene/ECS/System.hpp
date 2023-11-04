#pragma once

#include <Aka/Scene/ECS/World.hpp>
#include <Aka/Scene/Component/TagComponent.hpp>
#include <Aka/Scene/Component/TransformComponent.hpp>
#include <Aka/Scene/Component/HierarchyComponent.hpp>

namespace aka {
namespace ecs {

void onHierarchyRemove(entt::registry& registry, entt::entity entity)
{
	if (!registry.has<TransformComponent>(entity))
		return;
	TransformComponent& t = registry.get<TransformComponent>(entity);
	HierarchyComponent& h = registry.get<HierarchyComponent>(entity);
	// Restore local transform
	t.transform = h.inverseTransform * t.transform;
}

void onHierarchyUpdate(entt::registry& registry, entt::entity entity)
{
	// mark dirty to process update of flag
}

/*enum class UpdateType
{
	None,

	Update,
	PreRender,

	First = Update,
	Last = PreRender,
};


// Each world describe operations to perform on scene passed.
struct World
{
	World();
	~World();

	template <typename T, typename ...Args>
	void attach(EnumMask<UpdateType> mask, Args ...args);
	template <typename T>
	void detach();

	// Routine
	void update(Scene& scene);
	void preRender(Scene& scene);

private:
	Vector<System*> m_systems; // All systems created in this world
	Vector<System*> m_systemExecutions[EnumCount<UpdateType>()]; // All systems that run in this specific execution loop
};*/

// Only need functions though...
// but with class holding all logic for same component, its cleaner.

struct WorldEventListener
{
	WorldEventListener(WorldEventListener* listener);
};
struct World
{

};

class System
{
public:
	// Should not be attached to a single world...
	// Or should it ?
	// Or add world in a vector ?
	System(World& world) {}
	virtual ~System() {}
	virtual void update(World& scene, Time deltaTime) = 0;
	virtual void preRender(World& scene) = 0;
	virtual void postRender(World& scene) = 0;
};
// Custom physic system / havok / bullet physic system.
class PhysicSystem : System
{
	void update(Scene& scene, Time deltaTime) override
	{
		// Fixed update here manually... Dont need to dispatch this to whole app...

		// First iterate on all rigidbody to update accel & velocity
		// Then update transform component accordingly

		// then check all rigidbody for possible collision with colliders.
		// store all collisions somewhere in a vector.
		// React to this collision (if event collision or real collision)
		// OR emit event to be accessed by others
		// OR store all collisions within the rigid body component to be treated elsewhere.
		//		on each frame, clear collisions & recompute them.
		//		somehow we need to store if just entered or just left aswell...
	}
	void preRender(Scene& scene) override {}
};
// handle main camera & co.
class CameraSystem : System
{

};

class AnimatorSystem : System
{
	// Handle skeleton animations.
};

class HierarchySystem : System
{
	void onCreate(Scene& scene)
	{
		// This should be in system onCreate zz
		m_registry.on_destroy<HierarchyComponent>().connect<&onHierarchyRemove>();
		m_registry.on_update<HierarchyComponent>().connect<&onHierarchyUpdate>();
	}
	void onDestroy(Scene& scene)
	{

		m_registry.on_destroy<HierarchyComponent>().disconnect<&onHierarchyRemove>();
		m_registry.on_update<HierarchyComponent>().disconnect<&onHierarchyUpdate>();
	}
	void update(Scene& scene, Time deltaTime) override
	{
		// --- Update hierarchy transfom.
		// TODO only update if a hierarchy node or transform node has been updated. use dirtyTransform ?
		entt::registry& r = scene.registry();
		// Sort hierarchy to ensure correct order.
		// https://skypjack.github.io/2019-08-20-ecs-baf-part-4-insights/
		// https://wickedengine.net/2019/09/29/entity-component-system/
		r.sort<HierarchyComponent>([&r](const entt::entity lhs, entt::entity rhs) {
			const HierarchyComponent& clhs = r.get<HierarchyComponent>(lhs);
			const HierarchyComponent& crhs = r.get<HierarchyComponent>(rhs);
			return lhs < rhs && clhs.parent.handle() != rhs;
			});

		// Compute transforms
		auto transformView = world.registry().view<HierarchyComponent, TransformComponent>();
		for (entt::entity entity : transformView)
		{
			TransformComponent& t = r.get<TransformComponent>(entity);
			HierarchyComponent& h = r.get<HierarchyComponent>(entity);
			if (h.parent.valid())
			{
				mat4f localTransform = h.inverseTransform * t.transform;
				t.transform = h.parent.get<TransformComponent>().transform * localTransform;
				h.inverseTransform = mat4f::inverse(h.parent.get<TransformComponent>().transform);
			}
			else
			{
				h.inverseTransform = mat4f::identity();
			}
		}

		// --- Update light volumes.
		auto ligthView = world.registry().view<PointLightComponent>();
		for (entt::entity entity : ligthView)
		{
			PointLightComponent& l = r.get<PointLightComponent>(entity);
			// We are simply using 1/d2 as attenuation factor, and target for 5/256 as limit.
			l.radius = sqrt(l.intensity * 256.f / 5.f);
		}

		// --- Update arball camera
		// TODO move to separate camera system, rename this HierarchySystem
		auto cameraTransformView = world.registry().view<Transform3DComponent, Camera3DComponent>();
		for (entt::entity entity : cameraTransformView)
		{
			Transform3DComponent& transform = world.registry().get<Transform3DComponent>(entity);
			Camera3DComponent& camera = world.registry().get<Camera3DComponent>(entity);
			if (!camera.active || camera.controller == nullptr)
				continue;
			if (camera.controller->update(deltaTime))
			{
				world.registry().patch<Camera3DComponent>(entity);
			}
		}

	}
};

};
};