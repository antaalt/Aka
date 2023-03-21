#pragma once

#include <functional>
#include <memory>

#include <Aka/Core/Enum.h>
#include <Aka/Resource/Asset.h>
#include <Aka/Resource/AssetRegistry.h>
#include <Aka/Resource/ResourceArchive.h>

namespace aka {

class AssetImporter
{
public:
	AssetImporter() = default;
	virtual ~AssetImporter() = default;

	// Attach a new importer for a specific resource type
	static void attach(ResourceType type, AssetImporter* importer);
	// Import an asset in memory to asset folder and action on it
	static bool import(const Path& path, ResourceType type, std::function<void(Asset& asset)>&& callback);
	// Import an asset in memory to asset folder
	static bool import(const Path& path, ResourceType type);
	// Import an asset, save it and load it to memory
	static bool importAndLoad(const Path& path, ResourceType type, gfx::GraphicDevice* device);

	// Import a file using its type importer.
	virtual bool import(const Path& path, std::function<bool(Resource* resource)>&& callback) = 0;

private:
	static std::shared_ptr<AssetImporter> s_importers[EnumCount<ResourceType>()]; // List of all importers by resource
};

};