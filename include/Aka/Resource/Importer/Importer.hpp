#pragma once

#include <Aka/OS/Path.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

enum class ImportResult {
	Succeed,

	CouldNotReadFile,

	Failed,
};

class Importer {
public:
	virtual ImportResult import(AssetLibrary* _library, const Path & path) = 0;
	virtual ImportResult import(AssetLibrary* _library, const Blob& blob) = 0;
};

}