#pragma once

#include <Aka/Resource/Importer/Importer.hpp>

namespace aka {

class SpriteImporter : public Importer
{
public:
	ImportResult import(AssetLibrary * _library, const Path & path) override;
	ImportResult import(AssetLibrary * _library, const Blob & blob) override;
};

};