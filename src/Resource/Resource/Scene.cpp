#include <Aka/Resource/Resource/Scene.hpp>

#include <Aka/Resource/Archive/ArchiveScene.hpp>
#include <Aka/Resource/AssetLibrary.hpp>

#include <Aka/Renderer/Renderer.hpp>

#include <Aka/Scene/ECS/ecs.hpp>
#include <Aka/Scene/Component/TransformComponent.hpp>
#include <Aka/Scene/Component/HierarchyComponent.hpp>
#include <Aka/Scene/Component/CameraComponent.hpp>

namespace aka {

Scene::Scene() :
	Resource(ResourceType::Scene),
	m_mainCamera(ecs::EntityID::Invalid)
{
}
Scene::Scene(AssetID _id, const String& _name) :
	Resource(ResourceType::Scene, _id, _name),
	m_mainCamera(ecs::EntityID::Invalid)
{
}
Scene::~Scene()
{
	AKA_ASSERT(m_world.count() == 0, "Node destroy missing");
}

void Scene::fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer)
{
	const ArchiveScene& scene = _context.getArchive<ArchiveScene>(getID());

	m_bounds = scene.bounds;
#if 0
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
			allocatedComponent->fromArchive(*archiveComponent);
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
#else
	m_world.fromArchive(scene);
	m_bounds; // TODO:
	m_mainCamera;
#endif
}

void Scene::toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer)
{
	AKA_NOT_IMPLEMENTED;
	ArchiveScene& scene = _context.getArchive<ArchiveScene>(getID());
#if 1
	m_world.toArchive(scene);
	m_bounds;
	m_mainCamera;
#else
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
			component->toArchive(*archiveComponent);
			ArchiveSceneComponent archive;
			archive.id = component->getComponentID();
			archiveComponent->save(archive.archive);
			ComponentAllocator::freeArchive(archiveComponent);
			node.components.append(archive);
		}
		scene.nodes.append(node);
	}
#endif
}

void Scene::destroy_internal(AssetLibrary* _library, Renderer* _renderer)
{
	m_world.destroy();
}

void Scene::setMainCameraEntity(ecs::EntityID parent)
{
	AKA_ASSERT(m_world.has<CameraComponent>(parent), "");
	m_mainCamera = parent;
}

ecs::EntityID Scene::createEntity(ecs::EntityID parent, const char* name)
{
	ecs::EntityID entity =m_world.create(name);
	m_world.attach<TransformComponent>(entity);
	m_world.attach<HierarchyComponent>(entity);
	m_world.get<HierarchyComponent>(entity).parent = parent;
	return entity;
}

void Scene::destroyEntity(ecs::EntityID entity)
{
	//node->destroy();
	//AKA_ASSERT(m_root != node, "Cannot unlink root node.");
	//node->unlink();
	m_world.destroy(entity);
}

}