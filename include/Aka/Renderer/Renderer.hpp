#pragma once

#include <Aka/Core/Container/Vector.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Renderer/Instance.hpp>
#include <Aka/Renderer/Geometry.hpp>
#include <Aka/Renderer/DebugDraw/DebugDrawList.hpp>
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
static const uint32_t MaxGeometryBufferSize = 1 << 30; // 1Go

class Renderer final
{
public:
	Renderer(AssetLibrary* _library);
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) = delete;
	~Renderer();

	void create(gfx::GraphicDevice* _device);
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

	// -- Material
	MaterialHandle createMaterial();
	void updateMaterial(MaterialHandle handle, const color4f& color, TextureID albedo, TextureID normal);
	void destroyMaterial(MaterialHandle handle);

	TextureID allocateTextureID(gfx::TextureHandle texture);
	gfx::SamplerHandle getSampler(SamplerType type);

	// -- Layout
	gfx::ShaderBindingState getViewDescriptorSetLayout() { return m_viewDescriptorLayout; }
	gfx::ShaderBindingState getMaterialDescriptorSetLayout() { return m_materialDescriptorLayout; }
	gfx::ShaderBindingState getBindlessDescriptorSetLayout() { return m_bindlessDescriptorLayout; }

	// -- Viewport
	uint32_t getWidth() const { return m_width; }
	uint32_t getHeight() const { return m_height; }
private:
	friend class StaticMeshInstanceRenderer;
	friend class SkeletalMeshInstanceRenderer;
	gfx::BufferHandle getVertexGeometryBuffer() { return m_geometryVertexBuffer; }
	gfx::BufferHandle getIndexGeometryBuffer() { return m_geometryIndexBuffer; }
	gfx::BufferHandle getDataGeometryBuffer() { return m_geometryDataBuffer; }
	gfx::DescriptorSetHandle getMaterialDescriptorSet() { return m_materialSet; }
	gfx::DescriptorSetHandle getBindlessDescriptorSet() { return m_bindlessDescriptorSet; }
	uint32_t getMaterialIndex(MaterialHandle handle);
public:
	AssetLibrary* getLibrary() { return m_library; }
	gfx::GraphicDevice* getDevice() { return m_device; }
	PlatformWindow* getWindow() { return m_window; }
	ShaderRegistry* getShaderRegistry() { return m_shaders; }
	DebugDrawList& getDebugDrawList() { return m_debugDrawList; }
	gfx::RenderPassState getRenderPassState() { return m_backbufferRenderPassState; }
private:
	AssetLibrary* m_library;
	PlatformWindow* m_window;
	ShaderRegistry* m_shaders;
	gfx::GraphicDevice* m_device;
private:
	uint32_t m_width, m_height;
	InstanceRenderer* m_instanceRenderer[EnumCount<InstanceType>()];
private: // Backbuffer
	void createBackbuffer();
	void destroyBackbuffer();
	gfx::RenderPassState m_backbufferRenderPassState;
	gfx::TextureHandle m_depth;
	gfx::BackbufferHandle m_backbuffer;
	gfx::RenderPassHandle m_backbufferRenderPass;
private: // Views
	gfx::DescriptorPoolHandle m_viewDescriptorPool;
	HashMap<ViewHandle, View> m_views;
	uint32_t m_viewSeed = 0;
	bool m_viewDirty[gfx::MaxFrameInFlight];
private: // Material & textures
	bool m_materialDirty = 0;
	TextureID m_nextTextureID = (TextureID)0;
	uint32_t materialSeed;
	HashSet<TextureID> m_availableTexureID;
	gfx::DescriptorPoolHandle m_bindlessPool;
	gfx::DescriptorSetHandle m_bindlessDescriptorSet;
	Vector<MaterialData> m_materials;
	HashMap<MaterialHandle, uint32_t> m_materialIndex;
	gfx::BufferHandle m_materialBuffer;
	gfx::BufferHandle m_materialStagingBuffer;
	gfx::DescriptorSetHandle m_materialSet;
	gfx::DescriptorPoolHandle m_materialPool;
	gfx::SamplerHandle m_defaultSamplers[EnumCount<SamplerType>()];
	Vector<gfx::DescriptorUpdate> m_bindlessTextureUpdates;
private: // Layouts
	gfx::ShaderBindingState m_viewDescriptorLayout;
	gfx::ShaderBindingState m_materialDescriptorLayout;
	gfx::ShaderBindingState m_bindlessDescriptorLayout;
private:
	DebugDrawList m_debugDrawList;
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