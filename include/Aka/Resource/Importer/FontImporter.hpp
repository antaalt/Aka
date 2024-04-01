#pragma once

#include <Aka/Resource/Importer/Importer.hpp>

namespace aka {

class FontImporter : public Importer
{
public:
	FontImporter(AssetLibrary* _library);

	ImportResult import(const Path & path) override;
	ImportResult import(const Blob & blob) override;
};

};