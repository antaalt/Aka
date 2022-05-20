#include <Aka/Resource/Asset.h>

#include <Aka/OS/OS.h>
#include <Aka/Resource/AssetImporter.h>
#include <Aka/Resource/Archive/TextureArchive.h>

#include <memory>

namespace aka {

bool Asset::isLoaded() const
{
	return resource != nullptr;
}

bool Asset::load(gfx::GraphicDevice* device, bool clearBuildData)
{
	AKA_ASSERT(OS::File::exist(path), "File do not exist.");
	if (resource == nullptr)
	{
		resource = Resource::create(type);
		AKA_ASSERT(resource != nullptr, "Failed to create resource");
	}
	if (resource->getBuildData() == nullptr && resource->getRenderData() == nullptr)
	{
		std::unique_ptr<ResourceArchive> archive(resource->createResourceArchive());
		FileStream stream(path, FileMode::Read, FileType::Binary);
		resource->createBuildData();
		if (!archive->load(stream, resource->getBuildData()))
		{
			// Failed to load resource
			return false;
		}
	}
	if (resource->getRenderData() == nullptr)
	{
		resource->createRenderData(device, resource->getBuildData());
		if (clearBuildData)
			resource->destroyBuildData();
	}
	return true;
}

void Asset::unload(gfx::GraphicDevice* device)
{
	resource->destroyRenderData(device);
	resource->destroyBuildData();
	delete resource;
	resource = nullptr;
}

bool Asset::save()
{
	if (resource == nullptr || resource->getBuildData() == nullptr)
		return false;
	std::unique_ptr<ResourceArchive> archive(resource->createResourceArchive());
	FileStream stream(path, FileMode::Write, FileType::Binary);
	return archive->save(stream, resource->getBuildData());
}

};