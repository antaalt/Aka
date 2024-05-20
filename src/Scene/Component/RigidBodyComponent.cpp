#include <Aka/Scene/Component/RigidBodyComponent.hpp>
#include <Aka/Scene/Component/ColliderComponent.hpp>
#include <Aka/Renderer/Renderer.hpp>
#include <Aka/OS/Stream/MemoryStream.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveRigidBodyComponent::ArchiveRigidBodyComponent(ArchiveComponentVersionType _version) :
	ArchiveComponent(Component<RigidBodyComponent, ArchiveRigidBodyComponent>::getComponentID(), _version)
{
}

void ArchiveRigidBodyComponent::parse(BinaryArchive& archive)
{
	//archive.parse<AssetID>(assetID);
}

RigidBodyComponent::RigidBodyComponent(Node* node) :
	Component(node),
	m_velocity(0.f)
{
}
RigidBodyComponent::~RigidBodyComponent()
{
}

struct RigidBody;


// XPBD
struct Particle
{
	Particle(const vec3f& position, float mass) :
		position(position),
		velocity(vec3f(0.f)),
		massInverse(1.f / mass),
		previousPosition(0.f)
	{}
	// with 0 as mass & zero velocity, will not move (fixed vs dynamic)
	vec3f position; // m
	vec3f velocity; // m/s
	float massInverse; // 1/kg2

	vec3f previousPosition;
};
// Different for every shapes...
// https://www.toppr.com/guides/physics/system-of-particles-and-rotational-dynamics/moment-of-inertia/
mat3f computeSphereInertiaMatrix(float radius, float mass) {
	return mat3f::identity() * (2.f / 5.f * mass * radius);
}

// TODO: make component aswell
static Vector<Particle> particles;
void RigidBodyComponent::onBecomeActive(AssetLibrary* library, Renderer* _renderer)
{
	particles.append(Particle(vec3f(0.f, 10.f, 1.f), 5.f));
}
void RigidBodyComponent::onBecomeInactive(AssetLibrary* library, Renderer* _renderer)
{
	particles.clear();
}

void RigidBodyComponent::onRenderUpdate(AssetLibrary* library, Renderer* _renderer)
{
	DebugDrawList& list = _renderer->getDebugDrawList();

	uint32_t n = 0;
	for (RigidBodyComponent& body : getNode()->getAllocator().components<RigidBodyComponent>())
	{
		list.draw3DSphere(mat4f::TRS(body.m_position, body.m_orientation, vec3f(1.f)), color4f(0.f, 1.f, 0.f, 1.f));
		//Logger::info("Rigid body ", n++, " at pos ", body.m_position, " and rot ", body.m_orientation);
	}
	for (ColliderComponent& collider : getNode()->getAllocator().components<ColliderComponent>())
	{
		switch (collider.getShapeType())
		{
		case ColliderShapeType::Sphere:
			list.draw3DSphere(collider.getNode()->getWorldTransform(), color4f(1.f, 0.f, 0.f, 1.f));
			break;
		case ColliderShapeType::Plane:
			list.draw3DPlane(collider.getNode()->getWorldTransform(), color4f(1.f, 0.f, 0.f, 1.f));
			break;
		case ColliderShapeType::Cube:
			list.draw3DCube(collider.getNode()->getWorldTransform(), color4f(1.f, 0.f, 0.f, 1.f));
			break;
		default:
			break;
		}
	}
}

vec3f getExternalForces()
{
	const vec3f gravity(0.f, -9.81f, 0.f);
	const vec3f airResistance(0.f, 1.f, 0.f);
	return gravity + airResistance;
}
void RigidBodyComponent::onFixedUpdate(Time _deltaTime)
{
	// Dirty temp hack to simulate a system executing only once per frame.
	// With this dirty hack, onFixedUpdate change local transform, but next fixed update, we get world transform that is not cached yet.
	if (getNode()->getName() != "Sphere0")
		return;

	// https://matthias-research.github.io/pages/publications/PBDBodies.pdf
	const vec3f externalForces = getExternalForces();

	// XPBD

	// We are in a substep here (fixedUpdate)
	float dt = _deltaTime.seconds();
	float dt2 = dt * dt;
	{
		// First apply verlet integration
		{
			for (Particle& particle : particles)
			{
				particle.previousPosition = particle.position;
				particle.velocity += (externalForces * particle.massInverse) * dt;
				particle.position += particle.velocity * dt;
			}
			for (RigidBodyComponent& body : getNode()->getAllocator().components<RigidBodyComponent>())
			{
				// Get world position and cache it.
				body.m_position = mat4f::extractTranslation(body.getNode()->getWorldTransform());
				body.m_orientation = mat4f::extractRotation(body.getNode()->getWorldTransform());

				// Compute every thing related to physics.
				body.m_previousPosition = body.m_position;
				body.m_velocity += (externalForces * body.m_massInverse) * dt;
				body.m_position += body.m_velocity * dt;

				// Should we retrieve torque from other elements in contact here ?
				// TODO: externalTorque probably invalid here.
				const mat3f inertia = mat3f::inverse(body.m_inertiaInverse);
				vec3f externalTorque = inertia * body.m_angularVelocity; // T = Iw
				body.m_previousOrientation = body.m_orientation;
				body.m_angularVelocity += body.m_inertiaInverse * (externalTorque - vec3f::cross(body.m_angularVelocity, inertia * body.m_angularVelocity)) * dt;
				body.m_orientation += quatf(body.m_angularVelocity.x, body.m_angularVelocity.y, body.m_angularVelocity.z, 0.f) * 0.5f * dt;
				body.m_orientation = quatf::normalize(body.m_orientation);
			}
		}
		// Then we solve constraints, for now simply position constraints.
		{
			// We dont collider particles here.
			
			// Reset lagrange before computations.
			for (RigidBodyComponent& body : getNode()->getAllocator().components<RigidBodyComponent>())
			{
				body.m_lagrange = 0.f;
			}
			// Gather collisions pair
			// TODO: broad phase + narrow phase + octree + lot of things
			Vector<ContactData> collisions;
			PoolRange<ColliderComponent> range = getNode()->getAllocator().components<ColliderComponent>();
			for (PoolIterator<ColliderComponent> it = range.begin(); it != range.end(); it++)
			{
				ColliderComponent& collider = *it;
				if (!collider.getNode()->has<RigidBodyComponent>())
					continue; // Skip collider with rigidbody as main collider for testing.

				// Loop & check collisions
				if (it != range.end())
				{
					PoolIterator<ColliderComponent> itAfter = it; // Skip self
					for (PoolIterator<ColliderComponent> itOther = ++itAfter; itOther != range.end(); itOther++)
					{
						ColliderComponent& otherCollider = *itOther;
						ContactData contact = collider.computeContactData(otherCollider);
						if (contact.isColliding())
						{
							// TODO: avoid duplicate & sort by most prio object.
							collisions.append(contact);
						}
					}
				}
			}
			
			for (const ContactData& contact : collisions)
			{
				RigidBodyComponent::solvePositionConstraint(contact, dt);
			}
		}
		{
			// Then we apply position
			for (Particle& particle : particles)
			{
				particle.velocity = (particle.position - particle.previousPosition) / dt;
			}
			for (RigidBodyComponent& body : getNode()->getAllocator().components<RigidBodyComponent>())
			{
				body.m_velocity = (body.m_position - body.m_previousPosition) / dt;
				// TODO: check handedness of operations with quat * quat...
				quatf deltaOrientation = body.m_orientation * quatf::inverse(body.m_previousOrientation);
				body.m_angularVelocity = 2.f * vec3f(deltaOrientation.x, deltaOrientation.y, deltaOrientation.z) / dt;
				body.m_angularVelocity = deltaOrientation.w >= 0.f ? body.m_angularVelocity : -body.m_angularVelocity;

				// TODO: accumulate all transforms somewhere
				const Node* bodyParent = body.getNode()->getParent();
				const mat4f worldToLocal = bodyParent ? mat4f::inverse(bodyParent->getWorldTransform()) : mat4f::identity();
				const mat4f updatedTransform = worldToLocal * mat4f::TRS(body.m_position, body.m_orientation, mat4f::extractScale(body.getNode()->getWorldTransform()));
				body.getNode()->setLocalTransform(updatedTransform, true); // Should run this once per update instead of once per fixedUpdate.
			}
		}
		// TODO: solveVelocities
	}
}

void RigidBodyComponent::solvePositionConstraint(const ContactData& contact, float dt)
{
	// Should not use body.position here as we need relative positions. compute everything related to body 1 ? Or 
	Node& node1 = *contact.node1;
	Node& node2 = *contact.node2;
	const ColliderComponent& collider1 = node1.get<ColliderComponent>();
	const ColliderComponent& collider2 = node2.get<ColliderComponent>();
	AKA_ASSERT(node1.has<RigidBodyComponent>(), "First node should always be dynamic");
	RigidBodyComponent* rb1 = node1.has<RigidBodyComponent>() ? &node1.get<RigidBodyComponent>() : nullptr;
	RigidBodyComponent* rb2 = node2.has<RigidBodyComponent>() ? &node2.get<RigidBodyComponent>() : nullptr;

	// Positions of bodies in relative space.
	vec3f r1 = vec3f::zero();
	vec3f r2 = vec3f::zero();
	// Compute correction vector for r1 (r2 = -correction)
	// TODO: vector should be the other way around, something fishy...
	vec3f correction = vec3f::normalize(contact.surfaceHit2 - contact.surfaceHit1);
	// Cannot be negative cuz of dot
	float w1 = rb1 ? rb1->generalizedInverseMass(r1, correction) : 0.f;
	float w2 = rb2 ? rb2->generalizedInverseMass(r2, correction) : 0.f;

	// TODO: add support for compliance.
	// List of compliance here http://blog.mmacklin.com/2016/10/12/xpbd-slides-and-stiffness/
	float compliance = 0.f; // compliance of the constraint

	// For a plane, should take normal into account for sign.
	float magnitude = contact.penetration; // amplitude of the constraint
	float tildeCompliance = compliance / (dt * dt);
	// Computing Lagrange multiplier updates & accumulate it for same item.
	float lagrange = rb1 ? rb1->m_lagrange : 0.f; // TODO: what about rb2 lagrange ?
	float deltaLagrange = (-magnitude - tildeCompliance * lagrange) / (w1 + w2 + tildeCompliance);
	if (rb1)
		rb1->m_lagrange += deltaLagrange;
	// TODO: Do we need to store it in r2 aswell ?

	vec3f impulse = correction * deltaLagrange;
	if (rb1 && rb1->isDynamic())
	{
		vec3f rb1AngularImpulse = rb1->m_inertiaInverse * vec3f::cross(r1, impulse);
		rb1->m_position += impulse * rb1->m_massInverse;
		rb1->m_orientation += quatf(0.5f * rb1AngularImpulse.x, 0.5f * rb1AngularImpulse.y, 0.5f * rb1AngularImpulse.z, 0.f) * rb1->m_orientation;
		rb1->m_orientation = quatf::normalize(rb1->m_orientation);
	}
	if (rb2 && rb2->isDynamic())
	{
		vec3f rb2AngularImpulse = rb2->m_inertiaInverse * vec3f::cross(r2, impulse);
		rb2->m_position -= impulse * rb2->m_massInverse;
		rb2->m_orientation -= quatf(0.5f * rb2AngularImpulse.x, 0.5f * rb2AngularImpulse.y, 0.5f * rb2AngularImpulse.z, 0.f) * rb2->m_orientation;
		rb2->m_orientation = quatf::normalize(rb2->m_orientation);
	}

	//vec3f forces = rb1->m_lagrange * correction / (dt * dt);
}
void RigidBodyComponent::onUpdate(Time _deltaTime)
{
}
void RigidBodyComponent::fromArchive(const ArchiveRigidBodyComponent& archive)
{
	if (archive.mass == 0.f)
		m_massInverse = 0.f; // Static
	else
		m_massInverse = 1.f / archive.mass;
}

void RigidBodyComponent::toArchive(ArchiveRigidBodyComponent& archive)
{
	if (m_massInverse == 0.f)
		archive.mass = 0.f;
	else
		archive.mass = 1.f / m_massInverse;
}

float RigidBodyComponent::generalizedInverseMass(const vec3f& hitPoint, const vec3f& correction) const
{
	if (isDynamic())
	{
		vec3f axis = vec3f::cross(hitPoint, correction);
		return m_massInverse + vec3f::dot(axis, m_inertiaInverse * axis);
	}
	else
	{
		// Static can be seen as having infinite mass.
		return 0.f;
	}
}

bool RigidBodyComponent::isDynamic() const
{
	return m_massInverse > 0.f;
}

};