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
ArchiveLoadResult ArchiveScene::load_internal(ArchiveLoadContext& _context, BinaryArchive& _archive)
{
	// Before loading entities, load all components that will be used in the scene.
	_archive.read<point3f>(bounds.min);
	_archive.read<point3f>(bounds.max);

	uint32_t nbEntity = _archive.read<uint32_t>();
	for (uint32_t iEntity = 0; iEntity < nbEntity; iEntity++)
	{
		ArchiveSceneNode node{};
		node.name.resize(_archive.read<uint32_t>());
		_archive.read<char>(node.name.cstr(), node.name.length());
		node.name[node.name.length()] = '\0';

		_archive.read<mat4f>(node.transform);
		_archive.read<ArchiveSceneID>(node.parentID);

		uint32_t componentCount = _archive.read<uint32_t>();
		for (uint32_t i = 0; i < componentCount; i++)
		{
			ArchiveSceneComponent component;
			component.id = _archive.read<ComponentID>();
			component.archive.resize(_archive.read<uint32_t>());
			_archive.read<byte_t>(component.archive.data(), component.archive.size());
			node.components.append(component);
		}
		this->nodes.append(node);
	}
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveScene::save_internal(ArchiveSaveContext& _context, BinaryArchive& _archive)
{
	_archive.write<point3f>(bounds.min);
	_archive.write<point3f>(bounds.max);

	_archive.write<uint32_t>((uint32_t)this->nodes.size());
	for (size_t iEntity = 0; iEntity < this->nodes.size(); iEntity++)
	{
		ArchiveSceneNode& node = this->nodes[iEntity];

		_archive.write<uint32_t>((uint32_t)node.name.length());
		_archive.write<char>(node.name.cstr(), node.name.length());

		_archive.write<mat4f>(node.transform);
		_archive.write<ArchiveSceneID>(node.parentID);

		_archive.write<uint32_t>((uint32_t)node.components.size());
		for (size_t i = 0; i < node.components.size(); i++)
		{
			const ArchiveSceneComponent& component = node.components[i];
			_archive.write<ComponentID>(component.id);
			_archive.write<uint32_t>((uint32_t)component.archive.size());
			_archive.write<byte_t>(component.archive.data(), component.archive.size());
		}
	}

	return ArchiveSaveResult::Success;
}

ArchiveLoadResult ArchiveScene::load_dependency(ArchiveLoadContext& _context)
{
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveScene::save_dependency(ArchiveSaveContext& _context)
{
	return ArchiveSaveResult::Success;
}
void ArchiveScene::copyFrom(const Archive* _archive)
{
	AKA_ASSERT(_archive->id() == id(), "Invalid id");
	AKA_ASSERT(_archive->type() == type(), "Invalid type");
	AKA_ASSERT(_archive->version() == version(), "Invalid version");

	const ArchiveScene* archive = reinterpret_cast<const ArchiveScene*>(_archive);
	*this = *archive;
}

};