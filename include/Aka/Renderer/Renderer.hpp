#pragma once

#include <Aka/Core/Container/Vector.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Renderer/Instance.hpp>
#include <Aka/Renderer/Geometry.hpp>
#include <Aka/Renderer/Instance/SkeletalMeshInstance.hpp>
#include <Aka/Renderer/View.hpp>
#include <Aka/Core/Event.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>

namespace aka {

// TODO use sub namespace rnd

class AssetLibrary;
class InstanceRenderer;

enum class SamplerType
{
	Nearest,
	Bilinear,
	Trilinear,
	Anisotropic,

	First = Nearest,
	Last = Anisotropic,
};

enum class MaterialHandle : uint64_t { Invalid = (uint64_t)-1 };
enum class TextureID : uint32_t { Invalid = (uint32_t)-1 };
using TextureHandle = TextureID;

struct alignas(16) MaterialData
{
	float color[4];
	uint32_t albedoID;
	uint32_t normalID;
};

struct RendererMaterial
{
	MaterialData data;
};
static const uint32_t MaxViewCount = 5;
static const uint32_t MaxBindlessResources = 16536;
static const uint32_t MaxMaterialCount = 200;
static const uint32_t MaxGeometryBufferSize = 1 << 28;

class Renderer : EventListener<ShaderReloadedEvent>
{
public:
	Renderer(gfx::GraphicDevice* _device, AssetLibrary* _library);
	~Renderer();

	void create();
	void destroy();

	// -- Instances
	InstanceHandle createStaticMeshInstance(AssetID assetID);
	void updateStaticMeshInstanceTransform(InstanceHandle instance, const mat4f& transform);
	void destroyStaticMeshInstance(InstanceHandle instance);

	InstanceHandle createSkeletalMeshInstance(AssetID assetID);
	void updateSkeletalMeshInstanceTransform(InstanceHandle instance, const mat4f& transform);
	void updateSkeletalMeshBoneInstance(InstanceHandle instance, uint32_t index, const mat4f& transform);
	void destroySkeletalMeshInstance(InstanceHandle instance);

	// -- View
	ViewHandle createView(ViewType type);
	void updateView(ViewHandle handle, const mat4f& view, const mat4f& projection);
	void destroyView(ViewHandle view);

	// -- Allocate 
	// TODO move to separated class.
	static const size_t VertexDefaultAlignement = 16;
	static const size_t IndexDefaultAlignement = 16;
	GeometryBufferHandle allocateGeometryVertex(void* data, size_t size, size_t alignement = VertexDefaultAlignement);
	GeometryBufferHandle allocateGeometryIndex(void* data, size_t size, size_t alignement = IndexDefaultAlignement);
	GeometryBufferHandle allocateGeometryData(void* data, size_t size, size_t alignement = 0);
	void update(GeometryBufferHandle handle, const void* data, size_t size, size_t offset = 0);
	void deallocate(GeometryBufferHandle handle);
	gfx::BufferHandle getGeometryBuffer(GeometryBufferHandle handle);
	uint32_t getGeometryBufferOffset(GeometryBufferHandle handle);

	// -- Interactions
	void render(gfx::FrameHandle frame);
	void resize(uint32_t width, uint32_t height);
	void onReceive(const ShaderReloadedEvent& event);

	// -- Material
	MaterialHandle createMaterial();
	void updateMaterial(MaterialHandle handle, const color4f& color, TextureID albedo, TextureID normal);
	void destroyMaterial(MaterialHandle handle);

	TextureID allocateTextureID(gfx::TextureHandle texture);
	gfx::SamplerHandle getSampler(SamplerType type);

private:
	friend class StaticMeshInstanceRenderer;
	friend class SkeletalMeshInstanceRenderer;
	gfx::BufferHandle getVertexGeometryBuffer() { return m_geometryVertexBuffer; }
	gfx::BufferHandle getIndexGeometryBuffer() { return m_geometryIndexBuffer; }
	gfx::BufferHandle getDataGeometryBuffer() { return m_geometryDataBuffer; }
	gfx::DescriptorSetHandle getMaterialDescriptorSet() { return m_materialSet; }
	gfx::DescriptorSetHandle getBindlessDescriptorSet() { return m_bindlessDescriptorSet; }
	uint32_t getMaterialIndex(MaterialHandle handle);
	uint32_t getWidth() const { return m_width;  }
	uint32_t getHeight() const { return m_height; }
public:
	AssetLibrary* getLibrary() { return m_library; }
	gfx::GraphicDevice* getDevice() { return m_device; }
private:
	AssetLibrary* m_library;
	gfx::GraphicDevice* m_device;
private:
	uint32_t m_width, m_height;
	InstanceRenderer* m_instanceRenderer[EnumCount<InstanceType>()];
private: // Views
	gfx::DescriptorPoolHandle m_viewDescriptorPool;
	std::map<ViewHandle, View> m_views;
	uint32_t m_viewSeed = 0;
	bool m_viewDirty[gfx::MaxFrameInFlight];
private: // Material & textures
	bool m_materialDirty = 0;
	TextureID m_nextTextureID = (TextureID)0;
	uint32_t materialSeed;
	std::set<TextureID> m_availableTexureID;
	gfx::DescriptorPoolHandle m_bindlessPool;
	gfx::DescriptorSetHandle m_bindlessDescriptorSet;
	std::vector<MaterialData> m_materials;
	std::map<MaterialHandle, uint32_t> m_materialIndex;
	gfx::BufferHandle m_materialBuffer;
	gfx::BufferHandle m_materialStagingBuffer;
	gfx::DescriptorSetHandle m_materialSet;
	gfx::DescriptorPoolHandle m_materialPool;
	gfx::SamplerHandle m_defaultSamplers[EnumCount<SamplerType>()];
private: // Geometry
	size_t m_geometryVertexBufferAllocOffset = 0;
	size_t m_geometryIndexBufferAllocOffset = 0;
	size_t m_geometryDataBufferAllocOffset = 0;
	// Should use one single buffer as bindless
	gfx::BufferHandle m_geometryDataBuffer;
	gfx::BufferHandle m_geometryVertexBuffer;
	gfx::BufferHandle m_geometryIndexBuffer;
};



};