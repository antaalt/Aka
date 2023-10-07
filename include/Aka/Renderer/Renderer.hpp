#pragma once

#include <Aka/Core/Container/Vector.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Renderer/Instance.hpp>
#include <Aka/Renderer/View.hpp>
#include <Aka/Core/Event.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>

namespace aka {

// TODO use sub namespace rnd

class AssetLibrary;
class StaticMeshInstanceRenderer;

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

enum class GeometryBufferHandle : uint32_t { Invalid = (uint32_t)-1 };

struct GeometryBufferAllocation
{
	GeometryBufferHandle handle;
	//uint32_t bufferIndex; // using paged buffers
	uint32_t offset;
	uint32_t size;
};

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
	InstanceHandle createInstance(InstanceType _type, AssetID assetID);
	void updateInstanceTransform(InstanceType _type, InstanceHandle instance, const mat4f& transform);
	void destroyInstance(InstanceType _type, InstanceHandle instance);

	// -- View
	ViewHandle createView(ViewType type);
	void updateView(ViewHandle handle, const mat4f& view, const mat4f& projection);
	void destroyView(ViewHandle view);

	// -- Allocate 
	// TODO move to separated class.
	GeometryBufferHandle allocateGeometryVertex(void* data, size_t size);
	GeometryBufferHandle allocateGeometryIndex(void* data, size_t size);
	void update(const GeometryBufferHandle& handle, void* data, size_t size, size_t offset = 0);
	void deallocate(const GeometryBufferHandle& handle);
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
	gfx::BufferHandle getVertexGeometryBuffer() { return m_geometryVertexBuffer; }
	gfx::BufferHandle getIndexGeometryBuffer() { return m_geometryIndexBuffer; }
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
	StaticMeshInstanceRenderer* m_staticMeshRenderer;
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
	uint32_t m_geometryVertexBufferAllocOffset = 0;
	uint32_t m_geometryIndexBufferAllocOffset = 0;
	// Should use one single buffer as bindless
	gfx::BufferHandle m_geometryVertexBuffer;
	gfx::BufferHandle m_geometryIndexBuffer;
};



};