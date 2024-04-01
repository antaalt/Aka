#include <Aka/Resource/Archive/ArchiveScene.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveScene::ArchiveScene() : 
	ArchiveScene(AssetID::Invalid)
{
}
ArchiveScene::ArchiveScene(AssetID id) : 
	Archive(AssetType::Scene, id)
{
}
ArchiveParseResult ArchiveScene::parse(BinaryArchive& _archive)
{
	_archive.parse(this->bounds.min);
	_archive.parse(this->bounds.max);

	_archive.parse<ArchiveSceneNode>(this->nodes, [](BinaryArchive& archive, ArchiveSceneNode& node) {
		archive.parse(node.name);
		archive.parse(node.transform);
		archive.parse(node.parentID);
		archive.parse<ArchiveSceneComponent>(node.components, [](BinaryArchive& archive, ArchiveSceneComponent& component) {
			archive.parse<ComponentID>(component.id);
			archive.parse<byte_t>(component.archive);
		});
	});
	return ArchiveParseResult::Success;
}


ArchiveParseResult ArchiveScene::load_dependency(ArchiveLoadContext& _context)
{
	return ArchiveParseResult::Success;
}

};