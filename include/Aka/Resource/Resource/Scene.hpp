#pragma once 

#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Core/Container/String.h>
#include <Aka/Scene/ECS/World.hpp>

namespace aka {

struct ArchiveScene;
class AssetLibrary;
class Node3D;

class Scene : public Resource 
{
public:
	Scene();
	Scene(AssetID _id, const String& _name);
	~Scene();
private:
	void fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer) override;
	void toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer) override;
	void destroy_internal(AssetLibrary* library, Renderer* _renderer) override;

public:
	aabbox<> getBounds() const { return m_bounds; }

	ecs::World& getWorld() { return m_world; }

	const ecs::EntityID getMainCameraEntity() const { return m_mainCamera; }
	void setMainCameraEntity(ecs::EntityID node);
	
	ecs::EntityID createEntity(ecs::EntityID parent, const char* name);
	void destroyEntity(ecs::EntityID node);
private:
	aabbox<> m_bounds;
	ecs::World m_world;
	ecs::EntityID m_mainCamera;
};



};