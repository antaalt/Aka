#include <Aka/Scene/Node.hpp>

namespace aka {

static Pool<StaticMeshComponent> GlobalStaticMeshComponentPool;
static Pool<CameraComponent> GlobalCameraComponentPool;

template<> Pool<StaticMeshComponent>& GetGlobalComponentPool() { return GlobalStaticMeshComponentPool; }
template<> Pool<CameraComponent>& GetGlobalComponentPool() { return GlobalCameraComponentPool; }

Node::Node() : 
	m_name("Unknown"),
	m_dirtyMask(ComponentTypeMask::None)
{
}
Node::Node(const char* name) : 
	m_name(name),
	m_dirtyMask(ComponentTypeMask::None)
{
}
Node::~Node()
{
	AKA_ASSERT(m_components.size() == 0, "Missing components");
}

void Node::destroyComponents()
{
	for (Component* components : m_components)
	{
		// TODO components data might not have been destroyed here.
		components->onDetach();
		switch (components->type())
		{
		case ComponentType::StaticMeshComponent:
			GetGlobalComponentPool<StaticMeshComponent>().release(reinterpret_cast<StaticMeshComponent*>(components));
			break;
		case ComponentType::CameraComponent:
			GetGlobalComponentPool<CameraComponent>().release(reinterpret_cast<CameraComponent*>(components));
			break;
		default:
			AKA_ASSERT(false, "Missing type");
		}
	}
	m_components.clear();
}

};