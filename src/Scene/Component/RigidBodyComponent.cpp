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
struct RigidBody
{
	// Different for every shapes...
	// https://www.toppr.com/guides/physics/system-of-particles-and-rotational-dynamics/moment-of-inertia/
	static mat3f computeSphereInertiaMatrix(float radius, float mass) {
		return mat3f::identity() * (2.f / 5.f * mass * radius);
	}
	RigidBody(const vec3f& position, const quatf& orientation, float mass) :
		position(position),
		orientation(orientation),
		velocity(vec3f(0.f, 0.f, 1.f)),
		angularVelocity(vec3f(0.f)),
		inertiaInverse(mat3f::inverse(computeSphereInertiaMatrix(sphereRadius, mass))),
		massInverse(1.f / mass),
		previousPosition(0.f),
		previousOrientation(quatf::identity())
	{}

	vec3f position; // m
	quatf orientation; // rad
	vec3f velocity; // m/s
	vec3f angularVelocity; // rad/s
	mat3f inertiaInverse; // represent the center of gravity of object, based on center of mass.
	float massInverse; // 1/kg2

	vec3f previousPosition;
	quatf previousOrientation;
};
static Vector<Particle> particles;
static Vector<RigidBody> bodies;
void RigidBodyComponent::onBecomeActive(AssetLibrary* library, Renderer* _renderer)
{
	particles.append(Particle(vec3f(0.f, 10.f, 1.f), 5.f));
	bodies.append(RigidBody(vec3f(0.f, 3.f, 2.2f), quatf::identity(), 5.f));
	bodies.append(RigidBody(vec3f(0.f, 2.f, 2.f), quatf::identity(), 5.f));
	bodies.append(RigidBody(vec3f(0.f, 4.f, 1.3f), quatf::identity(), 5.f));
	bodies.append(RigidBody(vec3f(1.f, 4.f, 1.3f), quatf::identity(), 5.f));
	bodies.append(RigidBody(vec3f(1.5f, 5.f, 1.3f), quatf::identity(), 5.f));
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
			for (RigidBody& body : bodies)
			{
				// TODO: should handle collisions between instances here.
				// TODO: what about multi collisions ? How to handle them ?
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
					if (distance <= sphereRadius * 2.f)
					{
						// Collisions need to give to system: r1, r2 (hit points) and thats all ?
						vec3f normal = vec3f::normalize(otherBody.position - body.position);
						vec3f bodyHitPoint = body.position + normal * sphereRadius;
						vec3f otherBodyHitPoint = otherBody.position - normal * sphereRadius;
						vec3f bodyAxis = vec3f::cross(bodyHitPoint, normal);
						vec3f otherBodyAxis = vec3f::cross(otherBodyHitPoint, -normal);
						float w1 = body.massInverse + vec3f::dot(bodyAxis, body.inertiaInverse * bodyAxis);
						float w2 = otherBody.massInverse + vec3f::dot(otherBodyAxis, otherBody.inertiaInverse * otherBodyAxis);

						// List of compliance here http://blog.mmacklin.com/2016/10/12/xpbd-slides-and-stiffness/
						float compliance = 0.f; // compliance of the constraint
						float magnitude = distance; // amplitude of the constraint
						float tildeCompliance = compliance / dt2;
						// Computing Lagrange multiplier updates & accumulate it for same item.
						// TODO: store value in rigid body. Or cache it.
						float deltaLagrange = (-magnitude - tildeCompliance * lagrange) / (w1 + w2 + tildeCompliance);
						lagrange += deltaLagrange;

						vec3f impulse = normal * deltaLagrange;
						vec3f bodyAngularImpulse = body.inertiaInverse * vec3f::cross(bodyHitPoint, impulse);
						vec3f otherBodyAngularImpulse = otherBody.inertiaInverse * vec3f::cross(otherBodyHitPoint, impulse);
						// Current
						body.position += impulse * body.massInverse;
						body.orientation += quatf(0.5f * bodyAngularImpulse.x, 0.5f * bodyAngularImpulse.y, 0.5f * bodyAngularImpulse.z, 0.f) * body.orientation;
						body.orientation = quatf::normalize(body.orientation);
						// Other
						otherBody.position -= impulse * otherBody.massInverse;
						otherBody.orientation -= quatf(0.5f * otherBodyAngularImpulse.x, 0.5f * otherBodyAngularImpulse.y, 0.5f * otherBodyAngularImpulse.z, 0.f) * otherBody.orientation;
						otherBody.orientation = quatf::normalize(otherBody.orientation);

						vec3f forces = lagrange * normal / dt2;
					}
				}
				// Floor detection (Should be simply a plane collider)
				const float distFromFloor = body.position.y - sphereRadius;
				//const float distFromFloor = vec3f::distance(body.position, vec3f(body.position.x, 0.f, body.position.z));
				if (distFromFloor < 0.f)
				{
					// Apply impulse + rotation 
					vec3f hitPoint = vec3f(0.f, - sphereRadius, 0.f);
					vec3f correction = vec3f(0.f, 1.f, 0.f);
					vec3f axis = vec3f::cross(hitPoint, correction);
					// OPTIM: This w is zero for static & kinematic element (infinite mass)
					float w = body.massInverse + vec3f::dot(axis, body.inertiaInverse * axis);
					// No compliance as its 0
					float compliance = 0.f; // compliance of the constraint
					// Somehow setting magnitude negative fix a bit
					float magnitude = distFromFloor; // distance
					float tildeCompliance = compliance / dt2;
					float deltaLagrange = (-magnitude - tildeCompliance * lagrange) / (w + tildeCompliance);
					lagrange += deltaLagrange;

					vec3f impulse = correction * deltaLagrange;
					vec3f angularImpulse = body.inertiaInverse * vec3f::cross(hitPoint, impulse);
					body.position += impulse * body.massInverse;
					body.orientation += quatf(0.5f * angularImpulse.x, 0.5f * angularImpulse.y, 0.5f * angularImpulse.z, 0.0f) * body.orientation;
					body.orientation = quatf::normalize(body.orientation);
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