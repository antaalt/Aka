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
static const float sphereRadius = 0.5f;
struct RigidBody // TODO: component
{
	// Different for every shapes...
	// https://www.toppr.com/guides/physics/system-of-particles-and-rotational-dynamics/moment-of-inertia/
	static mat3f computeSphereInertiaMatrix(float radius, float mass) {
		return mat3f::identity() * (2.f / 5.f * mass * radius);
	}
	static RigidBody dynamic(const vec3f & position, const quatf & orientation, float mass)
	{
		RigidBody rb;
		rb.position = position;
		rb.orientation = orientation;
		rb.velocity = vec3f(0.f, 0.f, 1.f);
		rb.angularVelocity = vec3f(0.f);
		rb.inertiaInverse = computeSphereInertiaMatrix(sphereRadius, mass);
		rb.massInverse = 1.f / mass;
		rb.previousPosition = vec3f(0.f);
		rb.previousOrientation = quatf::identity();
		return rb;
	}
	static RigidBody fixed(const vec3f& position, const quatf& orientation)
	{
		RigidBody rb;
		rb.position = position;
		rb.orientation = orientation;
		rb.velocity = vec3f(0.f, 0.f, 1.f);
		rb.angularVelocity = vec3f(0.f);
		rb.inertiaInverse = mat3f::identity();
		rb.massInverse = 0.f;
		rb.previousPosition = vec3f(0.f);
		rb.previousOrientation = quatf::identity();
		return rb;
	}
	vec3f position; // m
	quatf orientation; // rad
	vec3f velocity; // m/s
	vec3f angularVelocity; // rad/s
	mat3f inertiaInverse; // represent the center of gravity of object, based on center of mass.
	float massInverse; // 1/kg2

	// Cache
	vec3f previousPosition;
	quatf previousOrientation;
	float lagrange = 0.f;

	bool isDynamic() const { return massInverse > 0.f; }

	float generalizedInverseMass(const vec3f& hitPoint, const vec3f& correction)
	{
		if (isDynamic())
		{
			vec3f axis = vec3f::cross(hitPoint, correction);
			return massInverse + vec3f::dot(axis, inertiaInverse * axis);
		}
		else
		{
			// Static can be seen as having infinite mass.
			return 0.f;
		}
	}


	static void solvePositionConstraint(ContactData& contact, float dt)
	{
		// Should not use body.position here as we need relative positions. compute everything related to body 1 ? Or 
		RigidBody& rb1 = *contact.rb1;
		RigidBody& rb2 = *contact.rb2;

		// Positions of bodies in relative space.
		vec3f r1 = vec3f::zero();
		vec3f r2 = vec3f::zero();
		// Compute correction vector for r1 (r2 = -correction)
		// TODO: vector should be the other way around, something fishy...
		vec3f correction = vec3f::normalize(contact.surfaceHit1 - contact.surfaceHit2);
		// Cannot be negative cuz of dot
		float w1 = rb1.generalizedInverseMass(r1, correction);
		float w2 = rb2.generalizedInverseMass(r2, correction);

		// TODO: add support for compliance.
		// List of compliance here http://blog.mmacklin.com/2016/10/12/xpbd-slides-and-stiffness/
		float compliance = 0.f; // compliance of the constraint

		// For a plane, should take normal into account for sign.
		float magnitude = contact.penetration; // amplitude of the constraint
		float tildeCompliance = compliance / (dt * dt);
		// Computing Lagrange multiplier updates & accumulate it for same item.
		// TODO: store value in rigid r1. Or cache it.
		// Do we need to store it in r2 aswell ?
		float deltaLagrange = (-magnitude - tildeCompliance * rb1.lagrange) / (w1 + w2 + tildeCompliance);
		rb1.lagrange += deltaLagrange;

		vec3f impulse = correction * deltaLagrange;
		if (rb1.isDynamic())
		{
			vec3f rb1AngularImpulse = rb1.inertiaInverse * vec3f::cross(r1, impulse);
			rb1.position += impulse * rb1.massInverse;
			rb1.orientation += quatf(0.5f * rb1AngularImpulse.x, 0.5f * rb1AngularImpulse.y, 0.5f * rb1AngularImpulse.z, 0.f) * rb1.orientation;
			rb1.orientation = quatf::normalize(rb1.orientation);
		}
		if (rb2.isDynamic())
		{
			vec3f rb2AngularImpulse = rb2.inertiaInverse * vec3f::cross(r2, impulse);
			rb2.position -= impulse * rb2.massInverse;
			rb2.orientation -= quatf(0.5f * rb2AngularImpulse.x, 0.5f * rb2AngularImpulse.y, 0.5f * rb2AngularImpulse.z, 0.f) * rb2.orientation;
			rb2.orientation = quatf::normalize(rb2.orientation);
		}

		vec3f forces = rb1.lagrange * correction / (dt * dt);
	}
};
static Vector<Particle> particles;
static Vector<RigidBody> bodies;
void RigidBodyComponent::onBecomeActive(AssetLibrary* library, Renderer* _renderer)
{
	particles.append(Particle(vec3f(0.f, 10.f, 1.f), 5.f));
	bodies.append(RigidBody::dynamic(vec3f(0.f, 3.f, 2.2f), quatf::identity(), 5.f));
	bodies.append(RigidBody::dynamic(vec3f(0.f, 2.f, 2.f), quatf::identity(), 5.f));
	bodies.append(RigidBody::dynamic(vec3f(0.f, 4.f, 1.3f), quatf::identity(), 5.f));
	bodies.append(RigidBody::dynamic(vec3f(1.f, 4.f, 1.3f), quatf::identity(), 5.f));
	bodies.append(RigidBody::dynamic(vec3f(1.5f, 5.f, 1.3f), quatf::identity(), 5.f));
}
void RigidBodyComponent::onBecomeInactive(AssetLibrary* library, Renderer* _renderer)
{
	particles.clear();
	bodies.clear();
}

void RigidBodyComponent::onRenderUpdate(AssetLibrary* library, Renderer* _renderer)
{
	DebugDrawList& list = _renderer->getDebugDrawList();

	uint32_t n = 0;
	for (RigidBody& body : bodies)
	{
		list.draw3DSphere(mat4f::TRS(body.position, body.orientation, vec3f(0.5f)), color4f(1.f, 0.f, 0.f, 1.f));
		Logger::info("Rigid body ", n++, " at pos ", body.position, " and rot ", body.orientation);
	}
}
void RigidBodyComponent::onFixedUpdate(Time _deltaTime)
{
	// https://matthias-research.github.io/pages/publications/PBDBodies.pdf
	const float mass = 1.f; // kg (f=ma)
	const vec3f gravity(0.f, -9.81f, 0.f);
	const vec3f airResistance(0.f, 1.f, 0.f);
	vec3f forces(0.f);
	forces += gravity;
	forces += airResistance;

	// XPBD

	// We are in a substep here (fixedUpdate)
	float dt = _deltaTime.seconds();
	float dt2 = dt * dt;
	{
		// TODO collect collisions pair...
		// First apply verlet integration
		{
			for (Particle& particle : particles)
			{
				particle.previousPosition = particle.position;
				particle.velocity += (forces / mass) * dt;
				particle.position += particle.velocity * dt;
			}
			for (RigidBody& body : bodies)
			{
				body.previousPosition = body.position;
				body.velocity += (forces / mass) * dt;
				body.position += body.velocity * dt;

				// Should we retrieve torque from other elements in contact here ?
				// TODO: externalTorque probably invalid here.
				const mat3f inertia = mat3f::inverse(body.inertiaInverse);
				vec3f externalTorque = inertia * body.angularVelocity; // T = Iw
				body.previousOrientation = body.orientation;
				body.angularVelocity += body.inertiaInverse * (externalTorque - vec3f::cross(body.angularVelocity, inertia * body.angularVelocity)) * dt;
				body.orientation += quatf(body.angularVelocity.x, body.angularVelocity.y, body.angularVelocity.z, 0.f) * 0.5f * dt;
				body.orientation = quatf::normalize(body.orientation);
			}
		}
		// Then we solve constraints, for now simply position constraints.
		{
			for (Particle& particle : particles)
			{
				// TODO: SolvePositions & resolve collisions
				// dont collide particles
			}
			// Reset lagrange before computations.
			for (RigidBody& body : bodies)
			{
				body.lagrange = 0.f;
			}
			for (RigidBody& body : bodies)
			{
				// https://gamedev.stackexchange.com/questions/176790/add-impulse-and-torque-to-a-cube
				// https://github.com/Jondolf/bevy_xpbd/blob/main/src/constraints/position_constraint.rs
				// Lagrange is shared between constraint for same object.
				float lagrange = 0.f;
				for (RigidBody& otherBody : bodies)
				{
					// skip self intersection
					if (&otherBody == &body)
						continue;
					const float distance = vec3f::distance(body.position, otherBody.position);
					if (distance < sphereRadius * 2.f)
					{
						vec3f normal = vec3f::normalize(otherBody.position - body.position);
						// this works if sphere radius is enough
						float magnitude = -(vec3f::distance(body.position, otherBody.position) - sphereRadius * 2.f);
						ContactData coll{
							&body,
							& otherBody,
							body.position + normal * sphereRadius,
							otherBody.position - normal * sphereRadius,
							normal,
							-normal,
							magnitude,
						};

						RigidBody::solvePositionConstraint(coll, dt);
					}
				}
				// Floor detection (Should be simply a plane collider)
				const float distFromFloor = body.position.y - sphereRadius;
				//const float distFromFloor = vec3f::distance(body.position, vec3f(body.position.x, 0.f, body.position.z));
				if (distFromFloor < 0.f)
				{
					static RigidBody floor = RigidBody::fixed(vec3f::zero(), quatf::identity());
					vec3f hitPoint = vec3f::down() * sphereRadius;
					// Position of floor is below...
					ContactData coll{
						&body,
						&floor,
						body.position + hitPoint,
						vec3f(body.position.x, 0.f, body.position.z),
						vec3f::down(),
						vec3f(0.f, 1.f, 0.f),
						-distFromFloor
					};

					RigidBody::solvePositionConstraint(coll, dt);
				}
			}
		}
		{
			// Then we apply position
			for (Particle& particle : particles)
			{
				particle.velocity = (particle.position - particle.previousPosition) / dt;
			}
			for (RigidBody& body : bodies)
			{
				body.velocity = (body.position - body.previousPosition) / dt;
				// TODO: check handedness of operations with quat * quat...
				quatf deltaOrientation = body.orientation * quatf::inverse(body.previousOrientation);
				body.angularVelocity = 2.f * vec3f(deltaOrientation.x, deltaOrientation.y, deltaOrientation.z) / dt;
				body.angularVelocity = deltaOrientation.w >= 0.f ? body.angularVelocity : -body.angularVelocity;
			}
		}
		// TODO: solveVelocities
	}
}
void RigidBodyComponent::onUpdate(Time _deltaTime)
{
}
void RigidBodyComponent::fromArchive(const ArchiveRigidBodyComponent& archive)
{
}

void RigidBodyComponent::toArchive(ArchiveRigidBodyComponent& archive)
{
}

};