#include <Aka/Resource/Resource/Scene.hpp>

#include <Aka/Resource/Archive/ArchiveScene.hpp>
#include <Aka/Resource/AssetLibrary.hpp>

#include <Aka/Scene/Component.hpp>
#include <Aka/Scene/Component/StaticMeshComponent.hpp>
#include <Aka/Renderer/Renderer.hpp>

#include <Aka/Scene/Node3D.hpp>

namespace aka {

Scene::Scene() :
	Resource(ResourceType::Scene),
	m_nodePool(),
	m_root(m_nodePool.acquire("RootNode"))
{
}
Scene::Scene(ResourceID _id, const String& _name) :
	Resource(ResourceType::Scene, _id, _name),
	m_nodePool(),
	m_root(m_nodePool.acquire("RootNode"))
{
}
Scene::~Scene()
{
	AKA_ASSERT(m_nodePool.count() == 0, "Node destroy missing");
	m_nodePool.release([this](Node3D& node) { Logger::warn(node.getName(), " was not destroyed"); });
}

void Scene::create_internal(AssetLibrary* _library, Renderer* _renderer, const Archive& _archive)
{
	AKA_ASSERT(_archive.type() == AssetType::Scene, "Invalid archive");
	const ArchiveScene& scene = reinterpret_cast<const ArchiveScene&>(_archive);

	m_bounds = scene.bounds;
	Vector<Node3D*> nodes;
	for (const ArchiveSceneEntity& entity : scene.entities)
	{
		Node3D* node = m_nodePool.acquire(entity.name.cstr());
		if (asBool(SceneComponentMask::Transform & entity.components))
		{
			ArchiveSceneID id = entity.id[EnumToIndex(SceneComponent::Transform)];
			AKA_ASSERT(id != ArchiveSceneID::Invalid, "No transform ID set");
			node->setLocalTransform(scene.transforms[toIntegral(id)].matrix);
			if (asBool(SceneComponentMask::Hierarchy & entity.components))
			{
				ArchiveSceneID idParent = entity.id[EnumToIndex(SceneComponent::Hierarchy)];
				if (idParent != ArchiveSceneID::Invalid)
				{
					Node3D* parent = nodes[toIntegral(idParent)];
					node->setParent(parent);
				}
				else
				{
					// To keep track of all child, always give them a parent.
					m_root->addChild(node);
				}
			}
			else
			{
				// To keep track of all child, always give them a parent.
				m_root->addChild(node);
			}
		}
		else
		{
			AKA_ASSERT(false, "NoTransform in node...");
		}
		if (asBool(SceneComponentMask::StaticMesh & entity.components))
		{
			ArchiveSceneID id = entity.id[EnumToIndex(SceneComponent::StaticMesh)];
			const ArchiveStaticMesh& mesh = scene.meshes[toIntegral(id)];
			StaticMeshComponent& s = node->attach<StaticMeshComponent>(mesh.id());
			s.setMesh(_library->load<StaticMesh>(_library->getResourceID(mesh.id()), mesh, _renderer));
			s.setInstance(_renderer->createInstance(mesh.id()));
		}
		//if (asBool(SceneComponentMask::PointLight & e.components))
		//{
		//	const ArchiveStaticMesh& mesh = scene.meshes[uint32_t(e.id)];
		//	mesh.getPath(); // TODO read cache with this somehow.
		//}
		nodes.append(node);
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
	AKA_NOT_IMPLEMENTED;
}

void recurseDestroy(Pool<Node3D>& pool, Node3D* root)
{
	for (uint32_t i = 0; i < root->getChildCount(); i++)
	{
		recurseDestroy(pool, root->getChild(i));
	}
	pool.release(root);
}

void Scene::destroy_internal(AssetLibrary* _library, Renderer* _renderer)
{
	m_root->destroy(_renderer->getDevice());
	recurseDestroy(m_nodePool, m_root);
}


Node3D* Scene::createChild(Node3D* parent, const char* name)
{
	Node3D* child = m_nodePool.acquire(name);
	parent->addChild(child);
	return child;
}

}