#pragma once 

#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Core/Container/String.h>

namespace aka {

class World;
struct ArchiveScene;
class AssetLibrary;

class Scene : public Resource 
{
public:
	Scene();
	Scene(ResourceID _id, const String& _name);
	~Scene();
private:
	void create_internal(AssetLibrary* library, gfx::GraphicDevice* _device, const Archive& _archive) override;
	void save_internal(AssetLibrary* library, gfx::GraphicDevice* _device, Archive& _archive) override;
	void destroy_internal(AssetLibrary* library, gfx::GraphicDevice* _device) override;

public:
	void update(aka::Time time, gfx::GraphicDevice* _device);
	void render(gfx::GraphicDevice* _device, gfx::Frame* _frame);

	aabbox<> getBounds() const { return m_bounds; }
	World& getWorld() { return *m_world; }
	const World& getWorld() const { return *m_world; }
private:
	aabbox<> m_bounds;
	World* m_world;
};

// Component
struct TagComponent
{
	String name;
};
struct Transform3DComponent 
{
	mat4f transform;
};



};