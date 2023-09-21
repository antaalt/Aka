#include <Aka/Resource/Archive/ArchiveScene.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveScene::ArchiveScene() : 
	ArchiveScene(AssetID::Invalid)
{
}
ArchiveScene::ArchiveScene(AssetID id) : 
	Archive(AssetType::Scene, id, getLatestVersion())
{
}
ArchiveLoadResult ArchiveScene::load_internal(ArchiveLoadContext& _context, BinaryArchive& _archive)
{
	// Before loading entities, load all components that will be used in the scene.
	_archive.read<point3f>(bounds.min);
	_archive.read<point3f>(bounds.max);
	{
		uint32_t nbTransform = _archive.read<uint32_t>();
		for (uint32_t iTransform = 0; iTransform < nbTransform; iTransform++)
		{
			ArchiveSceneTransform transform{};
			_archive.read<mat4f>(transform.matrix);
			this->transforms.append(transform);
		}
	}
	{
		// hierarchy only need an ID, do not store it.
	}
	{
		uint32_t nbStaticMesh = _archive.read<uint32_t>();
		for (uint32_t iMesh = 0; iMesh < nbStaticMesh; iMesh++)
		{
			AssetID assetID = _archive.read<AssetID>();
			this->meshes.append(ArchiveStaticMesh(assetID));
		}
	}

	uint32_t nbEntity = _archive.read<uint32_t>();
	for (uint32_t iEntity = 0; iEntity < nbEntity; iEntity++)
	{
		ArchiveSceneEntity entity{};
		entity.name.resize(_archive.read<uint32_t>());
		_archive.read<char>(entity.name.cstr(), entity.name.length());
		entity.name[entity.name.length()] = '\0';

		entity.components = _archive.read<SceneComponentMask>();

		for (uint32_t i = 0; i < EnumCount<SceneComponent>(); i++)
		{
			if (asBool(static_cast<SceneComponentMask>(1 << i) & entity.components))
			{
				entity.id[i] = _archive.read<ArchiveSceneID>();
			}
			else
			{
				entity.id[i] = ArchiveSceneID::Invalid;
			}
		}
		this->entities.append(entity);
	}

	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveScene::save_internal(ArchiveSaveContext& _context, BinaryArchive& _archive)
{
	_archive.write<point3f>(bounds.min);
	_archive.write<point3f>(bounds.max);
	{
		_archive.write<uint32_t>((uint32_t)this->transforms.size());
		for (size_t iTransform = 0; iTransform < this->transforms.size(); iTransform++)
		{
			_archive.write<mat4f>(this->transforms[iTransform].matrix);
		}
	}
	{
		// hierarchy only need an ID, do not store it.
	}
	{
		_archive.write<uint32_t>((uint32_t)this->meshes.size());
		for (size_t iMesh = 0; iMesh < this->meshes.size(); iMesh++)
		{
			_archive.write<AssetID>(this->meshes[iMesh].id());
		}
	}

	_archive.write<uint32_t>((uint32_t)this->entities.size());
	for (size_t iEntity = 0; iEntity < this->entities.size(); iEntity++)
	{
		ArchiveSceneEntity& entity = this->entities[iEntity];

		_archive.write<uint32_t>((uint32_t)entity.name.length());
		_archive.write<char>(entity.name.cstr(), entity.name.length());

		_archive.write<SceneComponentMask>(entity.components);

		for (uint32_t i = 0; i < EnumCount<SceneComponent>(); i++)
		{
			SceneComponentMask mask = static_cast<SceneComponentMask>(1 << i);
			if (asBool(mask & entity.components))
			{
				_archive.write<ArchiveSceneID>(entity.id[i]);
			}
		}
	}

	return ArchiveSaveResult::Success;
}

ArchiveLoadResult ArchiveScene::load_dependency(ArchiveLoadContext& _context)
{
	for (size_t iMesh = 0; iMesh < this->meshes.size(); iMesh++)
	{
		ArchiveLoadResult res = this->meshes[iMesh].load(_context);
		if (res != ArchiveLoadResult::Success)
			return res;
	}
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveScene::save_dependency(ArchiveSaveContext& _context)
{
	for (size_t iMesh = 0; iMesh < this->meshes.size(); iMesh++)
	{
		ArchiveSaveResult res = this->meshes[iMesh].save(_context);
		if (res != ArchiveSaveResult::Success)
			return res;
	}
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