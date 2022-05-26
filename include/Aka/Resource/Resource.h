#pragma once

#include <Aka/OS/Path.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/ResourceArchive.h>

namespace aka {

/*
*	Resource description
* 
*	This is an asset system for aka.
* 
*	A Resource represent the data in memory. Either CPU or GPU.
*	An Asset represent the resource file. All asset are stored in an AssetRegistry.
*	A ResourceArchive is responsible of loading an Asset and creating a Resource out of it.
*	An AssetImporter is responsible of loading an external file and saving it as an Asset.
*	For every Resource, there should be an AssetImporter and a ResourceArchive.
*
*	Asset::import([...]);
*
*	Asset.load([...]);
*
*	Resource.create([...]);
*
*	Enjoy then !
*/

enum class ResourceType
{
	Texture,
	Buffer,
	Sprite,
	Audio,
	Font,
	Mesh,
	Scene,

	Count,

	Unknown
};
template <typename T>
struct ResourceTrait
{
	static constexpr ResourceType type = ResourceType::Unknown;
};

struct BuildData {};
struct RenderData {};

struct ResourceArchive;

class Resource
{
public:
	Resource(ResourceType type);
	virtual ~Resource();

	static Resource* create(ResourceType type);
	static void destroy(Resource* resource);

	// Generate build data
	virtual void createBuildData() = 0;
	// Generate build data from render data.
	virtual void createBuildData(gfx::GraphicDevice* device, RenderData* data) = 0;
	// Destroy build data
	virtual void destroyBuildData();

	// Create render data from build data
	virtual void createRenderData(gfx::GraphicDevice* device, const BuildData* data) = 0;
	// Destroy render data
	virtual void destroyRenderData(gfx::GraphicDevice* device) = 0;

	// Create a resource archive for the resource with last version.
	virtual ResourceArchive* createResourceArchive() = 0;

	// Get render data 
	RenderData* getRenderData();
	// Get build data 
	BuildData* getBuildData();
	// Get const render data 
	const RenderData* getRenderData() const;
	// Get const build data 
	const BuildData* getBuildData() const;
	// Get type of resource
	ResourceType getType() const;
protected:
	ResourceType m_type;
	BuildData* m_buildData;
	RenderData* m_renderData;
};

};