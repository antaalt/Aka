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
			Component* allocatedComponent = ComponentAllocator::allocate(sceneNode, component.id);
			ArchiveComponent* archiveComponent = ComponentAllocator::allocateArchive(component.id);
			archiveComponent->load(component.archive);
			allocatedComponent->load(*archiveComponent);
			ComponentAllocator::freeArchive(archiveComponent);
			sceneNode->attach(allocatedComponent);
		}
		nodes.append(sceneNode);
	}
#if 0
	auto recurseDebug = std::function<void(Node*, uint32_t)>();
	recurseDebug = [&recurseDebug](Node* parent, uint32_t depth) {
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
	ArchiveScene& scene = reinterpret_cast<ArchiveScene&>(_archive);

	// Place all nodes in an array for serialization
	std::vector<Node*> nodes;
	auto recurseDebug = std::function<void(Node*)>();
	recurseDebug = [&recurseDebug, &nodes](Node* parent) {
		nodes.push_back(parent);
		// Serialized child
		for (uint32_t i = 0; i < parent->getChildCount(); i++)
		{
			recurseDebug(parent->getChild(i));
		}
	};
	recurseDebug(m_root);

	auto getParentID = [&nodes](Node* node) -> ArchiveSceneID {
		auto it = std::find(nodes.begin(), nodes.end(), node->getParent());
		AKA_ASSERT(it != nodes.end(), "");
		return ArchiveSceneID(it - nodes.begin());
	};

	scene.bounds = m_bounds;
	for (Node* sceneNode : nodes)
	{
		// Serialize node
		ArchiveSceneNode node;
		node.parentID = sceneNode->getParent() ? getParentID(sceneNode) : ArchiveSceneID::Invalid;
		node.name = sceneNode->getName();
		node.transform = sceneNode->getLocalTransform();
		// Serialize component
		for (auto pair : sceneNode->getComponentMap())
		{
			Component* component = pair.second;
			ArchiveComponent* archiveComponent = ComponentAllocator::allocateArchive(component->getComponentID());
			component->save(*archiveComponent);
			ArchiveSceneComponent archive;
			archive.id = component->getComponentID();
			archiveComponent->save(archive.archive);
			ComponentAllocator::freeArchive(archiveComponent);
			node.components.append(archive);
		}
		scene.nodes.append(node);
	}
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