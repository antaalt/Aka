#pragma once

#include <stdint.h>

#include <Aka/Core/Config.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/Scene/ECS/ecs.hpp>
#include <Aka/OS/Stream/MemoryStream.h>
#include <Aka/Resource/Archive/ArchiveScene.hpp>

namespace aka {
namespace ecs {

static size_t s_current_global = 0; // TODO move this to CPP to avoid duplicating across translation unit
template <typename Component>
size_t getRuntimeIndex()
{
	static size_t s_current = s_current_global++;
	return s_current;
}

// TODO put all this in a closed namespace. No need to expose it.
template <typename Component>
struct ComponentStorage
{
	EntityID entity;
	Component component;
};

class ComponentPoolBase
{
public:
	virtual void* allocate(EntityID entity) = 0;
	virtual void  deallocate(EntityID entity) = 0;
	virtual void* get(EntityID entity) = 0;
	virtual bool  has(EntityID entity) = 0;

	virtual void fromArchive(const ArchiveSceneComponent& archive) = 0;
	virtual void toArchive(ArchiveSceneComponent& archive) = 0;

	//const void* packed() const;
	//template <typename Component>
	//const ComponentStorage<Component>* raw() const;
};

template <typename Component>
class ComponentPool : public ComponentPoolBase
{
public:
	ComponentStorage<Component>* begin() { return m_packed.begin(); }
	ComponentStorage<Component>* end() { return m_packed.begin(); }

	void* get(EntityID entity)
	{
		AKA_ASSERT(entity != EntityID::Invalid, "Invalid entity ID");
		AKA_ASSERT(m_sparse.size() > static_cast<EntityType>(entity), "");
		AKA_ASSERT(m_sparse[static_cast<EntityType>(entity)] != (EntityType)-1, "");
		AKA_ASSERT(m_packed.size() > m_sparse[static_cast<EntityType>(entity)], "");

		return &m_packed[m_sparse[static_cast<EntityType>(entity)]].component;
	}
	bool has(EntityID entity)
	{
		AKA_ASSERT(entity != EntityID::Invalid, "Invalid entity ID");
		return m_sparse.size() > static_cast<EntityType>(entity) && m_sparse[static_cast<EntityType>(entity)] != (EntityType)-1 && m_packed.size() > m_sparse[static_cast<EntityType>(entity)];
	}
	void* allocate(EntityID entity)
	{
		if (m_sparse.size() <= static_cast<EntityType>(entity))
		{
			m_sparse.resize(static_cast<EntityType>(entity) + 1, EntityID::Invalid);
		}
		m_sparse[static_cast<EntityType>(entity)] = static_cast<uint16_t>(m_packed.size());
		m_packed.append(ComponentStorage<Component>{entity, Component{}});
		return &m_packed[m_sparse[static_cast<EntityType>(entity)]].component;
	}
	void deallocate(EntityID entity)
	{
		size_t index = m_sparse[static_cast<EntityType>(entity)];
		// This is heavy, could we handle it another way ?
		// We should defer deletion to avoid issues. also trigger event on deletion.
		m_packed.remove(m_packed.begin() + index);
		// TODO offset all others indices.
		m_sparse[static_cast<EntityType>(entity)] = EntityID::Invalid;
	}

	void fromArchive(const ArchiveSceneComponent& archive) override
	{
		MemoryReaderStream stream(archive.archive);
		BinaryArchiveReader reader(stream);

		Vector<ArchiveComponent<Component>> archives;

		reader.parse(m_sparse);
		reader.parse(archives);

		for (const ArchiveComponent<Component>& archive : archives)
		{
			ComponentStorage<Component> component;
			component.entity;
			archive.to(component.component);
			m_packed.append(component);
		}

	}
	void toArchive(ArchiveSceneComponent& archive) override
	{
		MemoryWriterStream stream(archive.archive);
		BinaryArchiveWriter writer(stream);

		Vector<ArchiveComponent<Component>> archives;
		for (ComponentStorage<Component>& storage : m_packed)
		{
			ArchiveComponent<Component> a;
			a.from(storage.component);
			archives.append(a);
		}

		writer.parse(m_sparse);
		writer.parse(archives);
	}
private:
	// sparse set
	Vector<uint32_t> m_vacants; // deallocated element that could be reallocated before end of frame.
	Vector<EntityType> m_sparse; // table with many invalid indices... Could use unordered map instead.
	Vector<ComponentStorage<Component>> m_packed; // storage is a component & an id.
};

};
};