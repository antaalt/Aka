#pragma once

#include <Aka/Resource/Importer/Importer.hpp>

namespace aka {

class AsepriteImporter : public Importer
{
public:
	AsepriteImporter(AssetLibrary* _library);

	ImportResult import(const Path & path) override;
	ImportResult import(const Blob & blob) override;
};

};