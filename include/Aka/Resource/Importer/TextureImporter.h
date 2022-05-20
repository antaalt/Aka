#pragma once

#include <Aka/Resource/AssetImporter.h>

namespace aka {

class TextureImporter : public AssetImporter
{
public:
	bool import(const Path& path, std::function<bool(Resource* resource)>&& callback) override;
};

};