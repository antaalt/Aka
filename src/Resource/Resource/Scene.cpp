#include <Aka/Resource/Resource/Scene.hpp>

#include <Aka/Resource/Archive/ArchiveScene.hpp>
#include <Aka/Resource/AssetLibrary.hpp>

#include <Aka/Scene/Component.hpp>
#include <Aka/Scene/Component/CameraComponent.hpp>
#include <Aka/Renderer/Renderer.hpp>

#include <Aka/Scene/Node.hpp>

namespace aka {

Scene::Scene() :
	Resource(ResourceType::Scene),
	m_allocator(),
	m_root(m_allocator.create("RootNode")),
	m_mainCamera(nullptr)
{
}
Scene::Scene(AssetID _id, const String& _name) :
	Resource(ResourceType::Scene, _id, _name),
	m_allocator(),
	m_root(m_allocator.create("RootNode")),
	m_mainCamera(nullptr)
{
}
Scene::~Scene()
{
}

void Scene::fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer)
{
	const ArchiveScene& scene = _context.getArchive<ArchiveScene>(getID());

	m_bounds = scene.bounds;
	Vector<Node*> nodes;
	for (const ArchiveSceneNode& node : scene.nodes)
	{
		Node* sceneNode = m_allocator.create(node.name.cstr());
		sceneNode->setLocalTransform(node.transform, true);
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
			ComponentBase* allocatedComponent = m_allocator.allocate(component.id, sceneNode);
			ArchiveComponent* archiveComponent = allocatedComponent->createArchiveBase();
			AKA_ASSERT(archiveComponent != nullptr && allocatedComponent != nullptr, "Component allocation failed.");
			archiveComponent->load(component.archive);
			allocatedComponent->fromArchiveBase(*archiveComponent);
			allocatedComponent->destroyArchiveBase(archiveComponent);
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

void Scene::toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer)
{
	ArchiveScene& scene = _context.getArchive<ArchiveScene>(getID());

	// Place all nodes in an array for serialization
	Vector<Node*> nodes;
	auto recurseDebug = std::function<void(Node*)>();
	recurseDebug = [&recurseDebug, &nodes](Node* parent) {
		nodes.append(parent);
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
			ComponentBase* component = pair.second;
			ArchiveComponent* archiveComponent = component->createArchiveBase();
			component->toArchiveBase(*archiveComponent);
			ArchiveSceneComponent archive;
			archive.id = component->getComponentID();
			archiveComponent->save(archive.archive);
			component->destroyArchiveBase(archiveComponent);
			node.components.append(archive);
		}
		scene.nodes.append(node);
	}
}

void recurseDestroy(NodeAllocator& allocator, Node* root)
{
	for (uint32_t i = 0; i < root->getChildCount(); i++)
	{
		recurseDestroy(allocator, root->getChild(i));
	}
	allocator.destroy(root);
}

void Scene::destroy_internal(AssetLibrary* _library, Renderer* _renderer)
{
	m_root->destroy(_library, _renderer);
	recurseDestroy(m_allocator, m_root);
}

void Scene::update(Time _deltaTime)
{
	m_allocator.visitComponentPools([=](ComponentBase& _component) {
		if (_component.getState() == ComponentState::Active)
		{
			_component.update(_deltaTime);
		}
	});
}
void Scene::fixedUpdate(Time _deltaTime)
{
	m_allocator.visitComponentPools([=](ComponentBase& _component) {
		if (_component.getState() == ComponentState::Active)
		{
			_component.fixedUpdate(_deltaTime);
		}
	});
}
void Scene::update(AssetLibrary* _library, Renderer* _renderer)
{
	// Activate & deactive required nodes & prepareUpdate
	// TODO: node activation & deactivation could be done from pool instead.
	m_allocator.visitNodes([=](Node& _node) {
		_node.updateComponentLifecycle(_library, _renderer);
		_node.prepareUpdate();
	});
	m_allocator.visitComponentPools([=](ComponentBase& _component) {
		if (_component.getState() == ComponentState::Active)
		{
			_component.renderUpdate(_library, _renderer);
		}
	});
	m_allocator.visitNodes([](Node& _node) {
		_node.finishUpdate();
	});
}

void Scene::setMainCameraNode(Node* parent)
{
	//AKA_ASSERT(m_nodePool.own(parent), "")
	AKA_ASSERT(parent->has<CameraComponent>(), "");
	m_mainCamera = parent;
}
void Scene::visitChildrens(std::function<void(Node*)> _callback) {
	m_root->visitChildrens(_callback);
}

Node* Scene::createChild(Node* parent, const char* name)
{
	if (parent == nullptr)
		parent = m_root;
	Node* child = m_allocator.create(name);
	parent->addChild(child);
	return child;
}

void Scene::destroyChild(Node* node)
{
	//node->destroy();
	//AKA_ASSERT(m_nodePool.own(node), "Do not own pool");
	//AKA_ASSERT(m_root != node, "Cannot unlink root node.");
	//node->unlink();
	m_allocator.destroy(node);
}

}