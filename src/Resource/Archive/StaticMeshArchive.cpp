#include <Aka/Resource/Archive/StaticMeshArchive.h>

#include <Aka/Core/Application.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/OS/Archive.h>
#include <Aka/OS/Logger.h>
#include <Aka/Resource/Resource/StaticMesh.h>

namespace aka {

bool StaticMeshArchive::load(Stream& stream, BuildData* data)
{
	BinaryArchive archive(stream);

	// Read header
	char sign[4];
	archive.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 'm')
		return false; // Invalid file
	StaticMeshArchiveVersion version = archive.read<StaticMeshArchiveVersion>();
	if (version > StaticMeshArchiveVersion::Latest)
		return false; // Incompatible version

	// Read mesh
	StaticMeshBuildData* meshData = reinterpret_cast<StaticMeshBuildData*>(data);

	AKA_NOT_IMPLEMENTED;

	return true;
}

bool StaticMeshArchive::save(Stream& stream, const BuildData* data)
{
	BinaryArchive archive(stream);

	// Write header
	char signature[4] = { 'a', 'k', 'a', 'm' };
	archive.write<char>(signature, 4);
	archive.write<StaticMeshArchiveVersion>(StaticMeshArchiveVersion::Latest);

	// Write mesh
	const StaticMeshBuildData* meshData = reinterpret_cast<const StaticMeshBuildData*>(data);
	
	AKA_NOT_IMPLEMENTED;

	return true;
}

};