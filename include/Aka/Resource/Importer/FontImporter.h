#pragma once

#include <Aka/Resource/AssetImporter.h>

namespace aka {

class FontImporter : public AssetImporter
{
public:
	bool import(const Path& path, std::function<bool(Resource* resource)>&& callback) override;
};

};