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
	m_root(new Node3D)
{
}
Scene::Scene(ResourceID _id, const String& _name) :
	Resource(ResourceType::Scene, _id, _name),
	m_root(new Node3D)
{
}
Scene::~Scene()
{
	delete m_root;
}

void Scene::create_internal(AssetLibrary* _library, Renderer* _renderer, const Archive& _archive)
{
	AKA_ASSERT(_archive.type() == AssetType::Scene, "Invalid archive");
	const ArchiveScene& scene = reinterpret_cast<const ArchiveScene&>(_archive);

	m_bounds = scene.bounds;
	Vector<Node3D*> nodes;
	for (const ArchiveSceneEntity& entity : scene.entities)
	{
		Node3D* node = m_root->addChild(entity.name.cstr());
		if (asBool(SceneComponentMask::Transform & entity.components))
		{
			ArchiveSceneID id = entity.id[EnumToIndex(SceneComponent::Transform)];
			node->setLocalTransform(scene.transforms[toIntegral(id)].matrix);
			if (asBool(SceneComponentMask::Hierarchy & entity.components))
			{
				ArchiveSceneID idParent = entity.id[EnumToIndex(SceneComponent::Hierarchy)];
				if (idParent != ArchiveSceneID::Invalid)
				{
					Node3D* parent = nodes[toIntegral(idParent)];
					node->setParent(parent);
				}
			}
		}
		else
		{
			AKA_ASSERT(false, "NoTransform in node...");
		}
		if (asBool(SceneComponentMask::StaticMesh & entity.components))
		{
			ArchiveSceneID id = entity.id[EnumToIndex(SceneComponent::StaticMesh)];
			StaticMeshComponent& s = node->attach<StaticMeshComponent>();
			const ArchiveStaticMesh& mesh = scene.meshes[toIntegral(id)];
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
}

void Scene::save_internal(AssetLibrary* library, Renderer* _renderer, Archive& _archive)
{
	AKA_NOT_IMPLEMENTED;
}

void Scene::destroy_internal(AssetLibrary* _library, Renderer* _renderer)
{
}

}