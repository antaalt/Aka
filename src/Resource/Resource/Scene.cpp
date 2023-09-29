#include <Aka/Resource/Resource/Scene.hpp>

#include <Aka/Resource/Archive/ArchiveScene.hpp>
#include <Aka/Resource/AssetLibrary.hpp>

#include <Aka/Scene/Component.hpp>
#include <Aka/Scene/Component/StaticMeshComponent.hpp>
#include <Aka/Renderer/Renderer.hpp>

#include <Aka/Scene/Node.hpp>

namespace aka {

Scene::Scene() :
	Resource(ResourceType::Scene),
	m_nodePool(),
	m_root(m_nodePool.acquire("RootNode"))
{
}
Scene::Scene(AssetID _id, const String& _name) :
	Resource(ResourceType::Scene, _id, _name),
	m_nodePool(),
	m_root(m_nodePool.acquire("RootNode"))
{
}
Scene::~Scene()
{
	AKA_ASSERT(m_nodePool.count() == 0, "Node destroy missing");
	m_nodePool.release([this](Node& node) { Logger::warn(node.getName(), " was not destroyed"); });
}

void Scene::create_internal(AssetLibrary* _library, Renderer* _renderer, const Archive& _archive)
{
	AKA_ASSERT(_archive.type() == AssetType::Scene, "Invalid archive");
	const ArchiveScene& scene = reinterpret_cast<const ArchiveScene&>(_archive);

	m_bounds = scene.bounds;
	Vector<Node*> nodes;
	for (const ArchiveSceneNode& node : scene.nodes)
	{
		Node* sceneNode = m_nodePool.acquire(node.name.cstr());
		sceneNode->setLocalTransform(node.transform);
		if (node.parentID != ArchiveSceneID::Invalid)
		{
			Node* parent = nodes[EnumToValue(node.parentID)];
			sceneNode->setParent(parent);
		}
		else
		{
			// To keep track of all child, always give them a parent.
			m_root->addChild(sceneNode);
		}
		for (const ArchiveSceneComponent& component : node.components)
		{
			Component* allocatedComponent = ComponentAllocator::allocate(component.id);
			ArchiveComponent* archiveComponent = ComponentAllocator::allocateArchive(component.id);
			archiveComponent->id = allocatedComponent->getComponentID();
			archiveComponent->size = 0;
			archiveComponent->version = 0;
			archiveComponent->parse(component.archive);
			allocatedComponent->load(*archiveComponent);
			ComponentAllocator::freeArchive(archiveComponent);
			sceneNode->attach(allocatedComponent);
		}
		nodes.append(sceneNode);
	}
#if 0
	auto recurseDebug = std::function<void(Node3D*, uint32_t)>();
	recurseDebug = [&recurseDebug](Node3D* parent, uint32_t depth) {
		String string;
		for (uint32_t i = 0; i < depth; i++)
			string += '\t';
		string += "- ";
		string += parent->getName();
		Logger::info(string);
		for (uint32_t i = 0; i < parent->getChildCount(); i++)
		{
			recurseDebug(parent->getChild(i), depth + 1);
		}
	};
	recurseDebug(m_root, 0);
#endif
}

void Scene::save_internal(AssetLibrary* library, Renderer* _renderer, Archive& _archive)
{
	AKA_ASSERT(_archive.type() == AssetType::Scene, "Invalid archive");
	ArchiveScene& sceneArchive = reinterpret_cast<ArchiveScene&>(_archive);
	sceneArchive;
	AKA_NOT_IMPLEMENTED;
}

void recurseDestroy(Pool<Node>& pool, Node* root)
{
	for (uint32_t i = 0; i < root->getChildCount(); i++)
	{
		recurseDestroy(pool, root->getChild(i));
	}
	pool.release(root);
}

void Scene::destroy_internal(AssetLibrary* _library, Renderer* _renderer)
{
	m_root->destroy(_library, _renderer);
	recurseDestroy(m_nodePool, m_root);
}


Node* Scene::createChild(Node* parent, const char* name)
{
	Node* child = m_nodePool.acquire(name);
	parent->addChild(child);
	return child;
}

void Scene::destroyChild(Node* node)
{
	//node->destroy();
	//AKA_ASSERT(m_nodePool.own(node), "Do not own pool");
	//AKA_ASSERT(m_root != node, "Cannot unlink root node.");
	//node->unlink();
	m_nodePool.release(node);
}

}