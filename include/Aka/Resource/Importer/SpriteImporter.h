#pragma once

#include <Aka/Resource/AssetImporter.h>

namespace aka {

class SpriteImporter : public AssetImporter
{
public:
	bool import(const Path& path, std::function<bool(Resource* resource)>&& callback) override;
};

};