#pragma once

#include <Aka/Resource/Importer/Importer.hpp>

namespace aka {

class TextureImporter : public Importer
{
public:
	TextureImporter(AssetLibrary* _library);

	ImportResult import(const Path & path) override;
	ImportResult import(const Blob & blob) override;
};

};