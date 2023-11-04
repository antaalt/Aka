#pragma once 
	
#include <Aka/Resource/Archive/Archive.hpp>

namespace aka {

enum class ArchiveSceneID : uint32_t { Invalid = (uint32_t)-1 };
enum class ArchiveSceneEntityID: uint16_t { Invalid = (uint32_t)-1 };
enum class ArchiveSceneComponentID : uint32_t { Invalid = (uint32_t)-1 };


struct ArchiveSceneComponent
{
	ArchiveSceneComponentID id;
	Vector<byte_t> archive; // Component pool with entities stored.

};

struct ArchiveScene : Archive 
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,

		Latest = ArchiveCreation
	};
	ArchiveScene();
	explicit ArchiveScene(AssetID id);

	aabbox<> bounds;
	Vector<ArchiveSceneEntityID> entities;
	Vector<ArchiveSceneComponent> components;

public: // helpers
	ArchiveSceneEntityID addEntity()
	{
		return entities.append(static_cast<ArchiveSceneEntityID>(entities.size()));
	}
	template <typename T>
	void addComponent(ArchiveSceneEntityID entity, const ecs::ArchiveComponent<T>& component)
	{
		ComponentID runtimeIndex = getRuntimeIndex<T>();
		if (EnumToValue(runtimeIndex) < components.size())
		{
			components.resize(EnumToValue(runtimeIndex) + 1);
		}
		// Write component to global archive.
		MemoryWriterStream stream(components[EnumToValue(runtimeIndex)].archive);
		BinaryArchiveWriter w(stream);
		c.parse(w);
	}

protected:
	ArchiveParseResult parse(BinaryArchive& path) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

};