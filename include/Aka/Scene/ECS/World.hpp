#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Scene/ECS/ComponentPool.hpp>

// CRC
#include <cstring>
#include <cstdint>
#include <iostream>


#define AKA_DECL_COMPONENT(ComponentName)																				\
struct ComponentName ## PoolAllocator : ::aka::ecs::ComponentPoolAllocator { \
	::aka::ecs::ComponentPoolBase* allocatePool() override { \
		return new ecs::ComponentPool<ComponentName>; \
	} \
	void destroyPool(::aka::ecs::ComponentPoolBase* pool) override { \
		delete pool; \
	} \
}; \
template <>																												\
struct ::aka::ecs::ArchiveComponentTrait<ComponentName>																				\
{																														\
	static constexpr const ::aka::ArchiveSceneComponentID id = static_cast<::aka::ArchiveSceneComponentID>(WSID(AKA_STRINGIFY(ComponentName))); \
	static constexpr const char* name = AKA_STRINGIFY(ComponentName);													\
	using PoolAllocator =  ComponentName ## PoolAllocator; \
	private:\
	static ::aka::ecs::ComponentRegister<ComponentName> reg; \
};

namespace aka {
namespace ecs {


// Lifecycle component
// 
// FrameBegin
// ComponentUpdate <- update & mark dirty & destroyed
// ComponentClear <- call onCreate / onUpdate / onDestroy on components.
// FrameEnd
// 
// This will add a lot of pool & co...
template <typename T>
struct ComponentAttached
{
	EntityID entity;
};
template <typename T>
struct ComponentDetached
{
	EntityID entity;
};

template <typename T>
struct ComponentUpdated // TODO dirty ?
{
	EntityID entity;
};
// TODO remove on next frame. world iterate at everyframe to remove these components (just clear vector)
struct EntityCreated
{
	EntityID entity;
	bool shouldBeDestroyed = false; // Destroy on next frame.
};
// TODO remove on next frame. world iterate at everyframe to remove these components (just clear vector)
struct EntityDestroyed
{
	EntityID entity;
	bool shouldBeDestroyed = false; // Destroy on next frame.
};
// 
struct EntityUpdated
{
	EntityID entity;
};


// Currently, we have
//	node with component
//		each node has n component & dispatch info to different data
//	
//	We should have a world holding all entity & component
//		Systems which are running over this world (scene)
//




template <typename T>
struct ArchiveComponentTrait
{
	static constexpr const ArchiveSceneComponentID id = 0;
	static constexpr const char* name = "Unknown";
	using PoolAllocator = ComponentPoolAllocator;
};


// --------------------------------------- CRC ----------------------------------

// Generate CRC lookup table
template <unsigned c, int k = 8>
struct f : f<((c & 1) ? 0xedb88320 : 0) ^ (c >> 1), k - 1> {};
template <unsigned c> struct f<c, 0> { enum { value = c }; };

#define A(x) B(x) B(x + 128)
#define B(x) C(x) C(x +  64)
#define C(x) D(x) D(x +  32)
#define D(x) E(x) E(x +  16)
#define E(x) F(x) F(x +   8)
#define F(x) G(x) G(x +   4)
#define G(x) H(x) H(x +   2)
#define H(x) I(x) I(x +   1)
#define I(x) f<x>::value ,

constexpr unsigned crc_table[] = { A(0) };

// Constexpr implementation and helpers
constexpr uint32_t crc32_impl(const char* p, size_t len, uint32_t crc) {
	return len ?
		crc32_impl(p + 1, len - 1, (crc >> 8) ^ crc_table[(crc & 0xFF) ^ *p])
		: crc;
}

constexpr uint32_t crc32(const char* data, size_t length) {
	return ~crc32_impl(data, length, ~0);
}

constexpr size_t strlen_c(const char* str) {
	return *str ? 1 + strlen_c(str + 1) : 0;
}

constexpr uint32_t WSID(const char* str) {
	return crc32(str, strlen_c(str));
}

// --------------------------------------- CRC ----------------------------------

struct ComponentPoolAllocator
{
protected:
	virtual ComponentPoolBase* allocatePool() = 0;
	virtual void destroyPool(ComponentPoolBase* pool) = 0;
public:
	template <typename T>
	static void registerPool()
	{

		s_componentID.insert(std::make_pair(ArchiveComponentTrait<T>::id, getRuntimeIndex<T>()));
		s_allocators.insert(std::make_pair(ArchiveComponentTrait<T>::id, new ArchiveComponentTrait<T>::PoolAllocator));

	}
	template <typename T>
	static void unregisterPool()
	{
		s_componentID.erase(ArchiveComponentTrait<T>::id);
		s_allocators.erase(ArchiveComponentTrait<T>::id);
	}
	static ComponentID getRuntimeIndex(ArchiveSceneComponentID componentID)
	{
		return s_componentID[componentID];
	}
	static ComponentPoolBase* allocate(ArchiveSceneComponentID componentID)
	{
		return s_allocators[componentID]->allocatePool();
	}
	static void deallocate(ArchiveSceneComponentID componentID, ComponentPoolBase* pool)
	{
		s_allocators[componentID]->destroyPool(pool);
	}
private:
	static std::map<ArchiveSceneComponentID, ComponentID> s_componentID;
	static std::map<ArchiveSceneComponentID, ComponentPoolAllocator*> s_allocators;
};

// But we need to have one per world.
// When creating world, we retrieve componentallocator & we iterate over all componentID to create componentpool (only on serialization, else, let runtime do its job)
template <typename Component>
struct ComponentRegister
{
	ComponentRegister()
	{
		ComponentPoolAllocator::registerPool<Component>();
	}
	~ComponentRegister()
	{
		ComponentPoolAllocator::unregisterPool<Component>();
	}
};



template <typename T>
struct ArchiveComponent
{
	// Component to archive
	void from(const T& component) {}
	// Archive to component
	void to(T& component) const {}
	// Archive declaration
	void parse(BinaryArchive& archive) { static_assert(false, "Missing implementation for type"); }
};

// Should plug an octree on it & divide transform in each bloc
// https://skypjack.github.io/2019-08-20-ecs-baf-part-4-insights/
class World // Scene...
{
public:
	// Create an entity
	// createNode (with transform & hierarchy) & pass parent
	// createEmpty (without transform & hierarchy) <- in octree
	EntityID create(const char* name)
	{
		EntityID id = (EntityID)m_entities.size();
		m_entities.append(id);
		// TODO create default component.
		// SHould we have a bitmask to hide if we require something
		return id;
	}
	// Destroy an entity and all its component
	void destroy(EntityID entity)
	{
		// TODO add to an available array
		m_entities[static_cast<std::underlying_type<EntityID>::type>(entity)] = EntityID::Invalid;
		attach<EntityDestroyed>(entity); // mark as destroyed. But also need to mark as destroyed for every allocated component.
		for (ComponentPoolBase* pool : m_pools)
		{
			// TODO generate event here...
			// return bool to tell if it was in pool or not.
			pool->deallocate(entity);
			// Here we check if we have listener & dispatch info.
			dispatcher->onDestroy(entity);
		}
	}
	void destroy()
	{
		for (ComponentPoolBase* pool : m_pools)
		{
			for (EntityID entity : pool)
			{
				dispatcher->onDestroy<ComponentType>(entity);
			}
			pool->deallocate();
		}
		entt::registry r;
		r.destroyed();
		// TODO would need some kind of event for destroyed component
	}

	// Attach a component to a given entity
	template <typename Component>
	Component& attach(EntityID entity)
	{
		size_t index = getIndex<Component>();
		if (m_pools.size() <= index)
		{
			m_pools.resize(index + 1);
			m_pools[index] = new ComponentPool<Component>;
		}
		Component& component = *static_cast<Component*>(m_pools[index]->allocate(entity));
		return component;
	}
	template <typename Component>
	Component& get(EntityID entity)
	{
		size_t index = getIndex<Component>();
		AKA_ASSERT(index < m_pools.size(), "Component not registered with attach...");
		return *static_cast<Component*>(m_pools[index]->get(entity));
	}
	template <typename Component>
	bool has(EntityID entity)
	{
		size_t index = getIndex<Component>();
		AKA_ASSERT(index < m_pools.size(), "Component not registered with attach...");
		return m_pools[index]->has(entity);
	}
	// Detach a component from an entity
	template <typename Component>
	void detach(EntityID entity)
	{
		size_t index = getIndex<Component>();
		AKA_ASSERT(index < m_pools.size(), "Component not registered with attach...");
		m_pools[index]->deallocate(entity);
	}
	// Sort entities with given functor
	// SHOULD NOT need this. Should have hierarchy sorting directly implemented within ecs system.
	void sort()
	{

	}

	uint32_t count() const { return (uint32_t)m_entities.size(); }

	struct EntityView
	{
		EntityView(Vector<EntityID>& entities) : m_entities(entities) {}

		using Iterator = Vector<EntityID>::Iterator;
		using ConstIterator = Vector<EntityID>::ConstIterator;
		
		Iterator begin() { return m_entities.begin(); }
		Iterator end() { return m_entities.end(); }
		ConstIterator begin() const { return m_entities.begin(); }
		ConstIterator end() const { return m_entities.end(); }
	private:
		Vector<EntityID>& m_entities;
	};
	// TODO template.
	//template <typename ...Component>
	struct ComponentView
	{
		ComponentView(World& world) :
			m_world(world),
			m_view(world.entities())
		{
		}
		friend class World;
		struct Iterator
		{
			explicit Iterator(World& world, EntityView::Iterator first, EntityView::Iterator last) :
				m_world(world),
				m_current(first),
				m_first(first),
				m_last(last)
			{
			}
			bool valid() const 
			{
				// TODO this based on template arg
			}
			Iterator& operator++()
			{
				while (!valid() && m_current != m_last) m_current++;;
				return *this;
			}
			Iterator operator++(int)
			{
				Iterator old = *this;
				++(*this);
				return old;
			}
			EntityID& operator*() { return *m_current; }
			const EntityID& operator*() const { return *m_current; }
			bool operator==(const Iterator& value) const { return value.m_current == m_current; }
			bool operator!=(const Iterator& value) const { return value.m_current != m_current; }
		private:
			World& m_world;
			EntityView::Iterator m_current;
			const EntityView::Iterator m_first;
			const EntityView::Iterator m_last;
		};
	private:
		World& m_world;
		EntityView m_view;
	};

	// Iterate over all entities. Should replace vector by some kind of EntityView
	EntityView entities() { return EntityView(m_entities); }
	// Iterate over all entities with these components
	template <typename ...Component>
	const Vector<EntityID>& components() const { return m_pools[getIndex<Component>()]->; }

	// TODO archiveworld
	// scene hold archiveworld + octree
	void fromArchive(const ArchiveScene& archive)
	{
		m_entities.reserve(archive.entities.size());
		for (ArchiveSceneEntityID id : archive.entities)
			m_entities.append(static_cast<EntityID>(id));

		for (const ArchiveSceneComponent& component : archive.components)
		{
			// Create pools from serialized ID on runtime.
			ComponentID runtimeIndex = ComponentPoolAllocator::getRuntimeIndex(component.id);
			if (static_cast<uint32_t>(runtimeIndex) >= m_pools.size())
			{
				m_pools.resize(static_cast<uint32_t>(runtimeIndex)+1, nullptr);
				m_pools[static_cast<uint32_t>(runtimeIndex)] = ComponentPoolAllocator::allocate(component.id);
			}
			m_pools[static_cast<uint32_t>(runtimeIndex)]->fromArchive(component);
		}
	}
	void toArchive(ArchiveScene& archive)
	{
		archive.entities.reserve(m_entities.size());
		for (EntityID id : m_entities)
			archive.entities.append(static_cast<ArchiveSceneEntityID>(id));
		for (ComponentPoolBase* pool : m_pools)
		{
			ArchiveSceneComponent sceneComponent;
			pool->toArchive(sceneComponent);
			archive.components.append(sceneComponent);
		}
	}

private:
	Vector<EntityID> m_entities;
	Vector<ComponentPoolBase*> m_pools;
};

}; // namespace ecs
}; // namespace aka