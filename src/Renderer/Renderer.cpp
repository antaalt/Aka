#include <Aka/Renderer/Renderer.hpp>

#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Core/Application.h>

namespace aka {

// SIMD ?
// issue with simd is alignement fit constant buffer, but not raw buffer.
// Should use structured buffer in this case.
namespace gpu {
// float
// double
using uint = uint32_t;
// int
struct alignas(16) uint2 {};
struct alignas(16) uint3 {};
struct alignas(16) uint4 {};
struct alignas(16) int2 {};
struct alignas(16) int3 {};
struct alignas(16) int4 {};
struct alignas(16) float2 {};
struct alignas(16) float3 {};
struct alignas(16) float4 {};
struct alignas(16) double2 {};
struct alignas(16) double3 {};
struct alignas(16) double4 {};
struct alignas(16) float4x4 { float4 cols[4]; };
struct alignas(16) float3x3 { float3 cols[3]; };
//struct alignas(16) float3x4 { float4 cols[3]; };
}

// TODO dynamic count.
static const uint32_t MaxInstanceCount = 500;
static const uint32_t MaxViewCount = 5;
static const uint32_t MaxBindlessResources = 16536;
static const uint32_t MaxMaterialCount = 200;
static const uint32_t MaxGeometryBufferSize = 1 << 26;

const char* getInstanceTypeName(InstanceType type)
{
	static const char* s_instanceTypeName[] = {
		"StaticMesh3D",
		"Sprite2D",
		"Text2D",
		"Text3D"
	};
	static_assert(countof(s_instanceTypeName) == EnumCount<InstanceType>());
	return s_instanceTypeName[EnumToIndex(type)];
}

struct InstanceRenderer
{
	virtual void prepareRender() = 0;
	virtual void render() = 0;
};

struct StaticMeshRenderer : InstanceRenderer
{

};

Renderer::Renderer(gfx::GraphicDevice* _device, AssetLibrary* _library) :
	m_device(_device),
	m_library(_library)
{
}
Renderer::~Renderer()
{
}

void Renderer::create()
{
	// Create instance buffer (need resize depending on count ?)
	ShaderRegistry* registry = Application::app()->program();
	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		// TODO: Should use JSON for this ? JSON that can be generated with a script reading all files in shaders folder (generating DB)
		const aka::Path ShaderVertexPath = aka::OS::cwd() + "../../../asset/shaders/shader.vert";
		const aka::Path ShaderFragmentPath = aka::OS::cwd() + "../../../asset/shaders/shader.frag";

		const aka::ShaderKey ShaderVertex = aka::ShaderKey().setPath(ShaderVertexPath).setType(aka::ShaderType::Vertex);
		const aka::ShaderKey ShaderFragment = aka::ShaderKey().setPath(ShaderFragmentPath).setType(aka::ShaderType::Fragment);

		InstanceRenderData& data = m_renderData[EnumToIndex(instanceType)];
		data.m_programKey = aka::ProgramKey();
		data.m_programKey.add(ShaderVertex).add(ShaderFragment);
		registry->add(data.m_programKey, m_device);

		getDevice()->getBackbufferSize(data.m_width, data.m_height);

		// These buffers are resized dynamically or preallocated ?
		data.m_instanceBuffer = m_device->createBuffer("InstanceBuffer", gfx::BufferType::Vertex, sizeof(InstanceData) * MaxInstanceCount, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);
		data.m_instanceBufferStaging = m_device->createBuffer("InstanceBuffer", gfx::BufferType::Vertex, sizeof(InstanceData) * MaxInstanceCount, gfx::BufferUsage::Staging, gfx::BufferCPUAccess::ReadWrite);
	}
	gfx::ShaderBindingState bindings{};
	bindings.add(gfx::ShaderBindingType::UniformBuffer, gfx::ShaderMask::Vertex);
	m_viewBuffers = m_device->createBuffer("ViewBuffer", gfx::BufferType::Uniform, sizeof(ViewData) * MaxViewCount, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);

	// Bindless
	gfx::ShaderBindingState bindlessBindings{};
	bindlessBindings.add(gfx::ShaderBindingType::SampledImage, gfx::ShaderMask::Fragment, gfx::ShaderBindingFlag::Bindless, MaxBindlessResources);
	m_bindlessPool = getDevice()->createDescriptorPool("BindlessPool", bindlessBindings, MaxBindlessResources);
	m_bindlessDescriptorSet = getDevice()->allocateDescriptorSet("BindlessSet", bindlessBindings, m_bindlessPool);

	// View
	gfx::DescriptorPoolHandle pool = m_viewDescriptorPool.append(getDevice()->createDescriptorPool("ViewDescriptorPool", bindings, MaxViewCount));
	m_viewDescriptorSet.append(getDevice()->allocateDescriptorSet("ViewDescSet", bindings, pool));

	Vector<gfx::DescriptorUpdate> desc;
	desc.append(gfx::DescriptorUpdate::uniformBuffer(0, 0, m_viewBuffers, 0, sizeof(ViewData)));
	m_device->update(m_viewDescriptorSet[0], desc.data(), desc.size());

	m_defaultSamplers[EnumToIndex(SamplerType::Nearest)] = m_device->createSampler(
		"Sampler",
		gfx::Filter::Nearest, gfx::Filter::Nearest,
		gfx::SamplerMipMapMode::None,
		gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat,
		1.0
	);
	m_defaultSamplers[EnumToIndex(SamplerType::Bilinear)] = m_device->createSampler(
		"Sampler",
		gfx::Filter::Linear, gfx::Filter::Linear,
		gfx::SamplerMipMapMode::None,
		gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat,
		1.0
	);
	m_defaultSamplers[EnumToIndex(SamplerType::Trilinear)] = m_device->createSampler(
		"Sampler",
		gfx::Filter::Linear, gfx::Filter::Linear,
		gfx::SamplerMipMapMode::Linear,
		gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat,
		1.0
	);
	m_defaultSamplers[EnumToIndex(SamplerType::Anisotropic)] = m_device->createSampler(
		"Sampler",
		gfx::Filter::Linear, gfx::Filter::Linear,
		gfx::SamplerMipMapMode::Linear,
		gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat,
		1.0
	);
	// Material
	m_materialStagingBuffer = getDevice()->createBuffer("MaterialBuffer", gfx::BufferType::Storage, sizeof(MaterialData) * MaxMaterialCount, gfx::BufferUsage::Staging, gfx::BufferCPUAccess::ReadWrite);
	m_materialBuffer = getDevice()->createBuffer("MaterialBuffer", gfx::BufferType::Storage, sizeof(MaterialData) * MaxMaterialCount, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);
	
	gfx::ShaderBindingState materialBindings{};
	materialBindings.add(gfx::ShaderBindingType::StorageBuffer, gfx::ShaderMask::Vertex | gfx::ShaderMask::Fragment);
	m_materialPool = getDevice()->createDescriptorPool("MaterialDescriptorPool", materialBindings, MaxMaterialCount);
	m_materialSet = getDevice()->allocateDescriptorSet("MaterialdescriptorSet", materialBindings, m_materialPool);

	Vector<gfx::DescriptorUpdate> updates;
	updates.append(gfx::DescriptorUpdate::storageBuffer(0, 0, m_materialBuffer, 0, sizeof(MaterialData) * MaxMaterialCount));
	getDevice()->update(m_materialSet, updates.data(), updates.size());

	// Geometry
	m_geometryVertexBuffer = getDevice()->createBuffer("GeometryVertexBuffer", gfx::BufferType::Vertex, MaxGeometryBufferSize, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);
	m_geometryIndexBuffer = getDevice()->createBuffer("GeometryIndexBuffer", gfx::BufferType::Index, MaxGeometryBufferSize, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);

	createRenderPass();
}

void Renderer::destroy()
{
	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		InstanceRenderData& data = m_renderData[EnumToIndex(instanceType)];
		m_device->destroy(data.m_instanceBuffer);
		m_device->destroy(data.m_instanceBufferStaging);
	}
	for (SamplerType samplerType : EnumRange<SamplerType>())
	{
		m_device->destroy(m_defaultSamplers[EnumToIndex(samplerType)]);
	}
	destroyRenderPass();
	m_device->destroy(m_materialStagingBuffer);
	m_device->destroy(m_materialBuffer);
	m_device->free(m_materialSet);
	m_device->destroy(m_materialPool);
	m_device->destroy(m_viewBuffers);
	m_device->free(m_bindlessDescriptorSet);
	m_device->destroy(m_bindlessPool);
	m_device->destroy(m_geometryVertexBuffer);
	m_device->destroy(m_geometryIndexBuffer);
	for (gfx::DescriptorSetHandle handle : m_viewDescriptorSet)
		m_device->free(handle);
	for (gfx::DescriptorPoolHandle handle : m_viewDescriptorPool)
		m_device->destroy(handle);
}

void Renderer::createRenderPass()
{
	ShaderRegistry* registry = Application::app()->program();

	m_backbufferRenderPass = m_device->createBackbufferRenderPass();
	m_backbuffer = m_device->createBackbuffer(m_backbufferRenderPass);
	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		InstanceRenderData& data = m_renderData[EnumToIndex(instanceType)];
		gfx::ProgramHandle programHandle = registry->get(data.m_programKey);

		// For now, we hack these as its difficult to generate them from shader reflection.
		// Should not use shader reflection and maybe have some kind of api 
		// such as shader->isCompatible(bindings) based on reflection.
		// This way, we can disable it.
		const_cast<gfx::Program*>(m_device->get(programHandle))->sets[2].bindings[0].count = MaxBindlessResources;
		const_cast<gfx::Program*>(m_device->get(programHandle))->sets[2].bindings[0].flags = gfx::ShaderBindingFlag::Bindless;
		// Create pipeline
		data.m_pipeline = m_device->createGraphicPipeline(
			"Graphic pipeline",
			programHandle,
			gfx::PrimitiveType::Triangles,
			m_device->get(m_backbufferRenderPass)->state,
			gfx::VertexState{}.add(StaticVertex::getState()).add(InstanceData::getState()),
			gfx::ViewportState{}.size(data.m_width, data.m_height),
			gfx::DepthStateLessEqual,
			gfx::StencilStateDefault,
			gfx::CullStateDefault,
			gfx::BlendStateDefault,
			gfx::FillStateFill
		);
	}

}
void Renderer::destroyRenderPass()
{
	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		InstanceRenderData& data = m_renderData[EnumToIndex(instanceType)];
		m_device->destroy(data.m_pipeline);
	}
	m_device->destroy(m_backbufferRenderPass);
	m_device->destroy(m_backbuffer);
}

InstanceType getInstanceTypeFromAssetType(AssetType assetType)
{
	switch (assetType)
	{
	case aka::AssetType::StaticMesh:
		return InstanceType::StaticMesh3D;
	default:
		AKA_NOT_IMPLEMENTED;
		return InstanceType::Unknown;
	}
}

InstanceHandle Renderer::createInstance(AssetID assetID)
{
	if (assetID == AssetID::Invalid)
		return InstanceHandle::Invalid;
	AssetInfo assetInfo = m_library->getAssetInfo(assetID);
	InstanceType instanceType = getInstanceTypeFromAssetType(assetInfo.type);
	// Generate handle
	size_t hash = hash::fnv(&assetID, sizeof(AssetID));
	hash::combine(hash, instanceType);
	hash::combine(hash, m_instanceSeed++);
	InstanceHandle instanceHandle = static_cast<InstanceHandle>(hash);

	Instance instance{};
	instance.type = instanceType;
	instance.assetID = assetID;
	instance.mask = ViewTypeMask::Color;
	instance.transform = mat4f::identity();

	AKA_ASSERT(m_instances.find(instanceHandle) == m_instances.end(), "Hash collision");

	// Insert data
	m_instances.insert(std::make_pair(instanceHandle, instance));

	m_instancesDirty |= instanceType;

	return instanceHandle;
}

void Renderer::updateInstanceTransform(InstanceHandle instanceHandle, const mat4f& transform)
{
	if (instanceHandle == InstanceHandle::Invalid)
		return;
	auto it = m_instances.find(instanceHandle);
	AKA_ASSERT(it != m_instances.end(), "");

	it->second.transform = transform;

	m_instancesDirty |= it->second.type;
}


void Renderer::destroyInstance(InstanceHandle instanceHandle)
{
	if (instanceHandle == InstanceHandle::Invalid)
		return;

	auto it = m_instances.find(instanceHandle);
	m_instancesDirty |= it->second.type;
	m_instances.erase(instanceHandle);
}


ViewHandle Renderer::createView(ViewType viewType)
{
	ViewHandle handle = static_cast<ViewHandle>(m_views.size());
	View& view = m_views.emplace();
	view.type = viewType;
	view.data = ViewData{};

	m_viewDirty = true;

	return handle;
}

void Renderer::updateView(ViewHandle handle, const mat4f& view, const mat4f& projection)
{
	m_views[EnumToValue(handle)].data.projection = projection;
	m_views[EnumToValue(handle)].data.view = view;

	m_viewDirty = true;
}

void Renderer::destroyView(ViewHandle view)
{
	// Cant remove it, might break indexation, use map instead ?
	m_views[EnumToValue(view)];

	m_viewDirty = true;
}

GeometryBufferHandle Renderer::allocateGeometryVertex(void* data, size_t size)
{
	uint32_t uSize = static_cast<uint32_t>(size);
	// TODO alignement & better allocator.
	AKA_ASSERT(uSize + m_geometryVertexBufferAllocOffset <= MaxGeometryBufferSize, "Out of bounds");
	uint32_t idBitmask = (1U << 31); // Mark it as vertex
	GeometryBufferHandle handle = static_cast<GeometryBufferHandle>(m_geometryVertexBufferAllocOffset | idBitmask);
	getDevice()->upload(m_geometryVertexBuffer, data, m_geometryVertexBufferAllocOffset, uSize);
	m_geometryVertexBufferAllocOffset += uSize;
	return handle;
}
GeometryBufferHandle Renderer::allocateGeometryIndex(void* data, size_t size)
{
	uint32_t uSize = static_cast<uint32_t>(size);
	// TODO alignement & better allocator.
	AKA_ASSERT(uSize + m_geometryIndexBufferAllocOffset <= MaxGeometryBufferSize, "Out of bounds");
	uint32_t idBitmask = 0;
	GeometryBufferHandle handle = static_cast<GeometryBufferHandle>(m_geometryIndexBufferAllocOffset | idBitmask);
	getDevice()->upload(m_geometryIndexBuffer, data, m_geometryIndexBufferAllocOffset, uSize);
	m_geometryIndexBufferAllocOffset += uSize;
	return handle;
}

void Renderer::update(const GeometryBufferHandle& handle, void* data, size_t size, size_t offset)
{
	AKA_NOT_IMPLEMENTED;
}

void Renderer::deallocate(const GeometryBufferHandle& handle)
{
	// Nothing, linear allocator for now
}

gfx::BufferHandle Renderer::getGeometryBuffer(GeometryBufferHandle handle)
{
	if ((static_cast<uint32_t>(handle) >> 31) & 0x1)
		return m_geometryVertexBuffer;
	return m_geometryIndexBuffer;
}

uint32_t Renderer::getGeometryBufferOffset(GeometryBufferHandle handle)
{
	return static_cast<uint32_t>(handle) & bitmask(31); // Remove id bit
}


void Renderer::render(gfx::Frame* frame)
{
	gfx::CommandList* cmd = m_device->getGraphicCommandList(frame);

	// TODO should run through all views ?
	if (m_views.size() == 0)
		return;

	View& view = m_views[m_currentView];

	if (m_viewDirty)
	{
		ViewData ubo;
		ubo.view = view.data.view;
		ubo.projection = view.data.projection;
		m_device->upload(m_viewBuffers, &ubo, 0, sizeof(ViewData));
		m_viewDirty = false;
	}

	if (!m_instancesDirty.empty() || m_materialDirty)
	{
		// Group all instances by type & asset for batching draw call.
		std::map<AssetID, Vector<mat4f>> assetInstances[EnumCount<InstanceType>()];
		for (const std::pair<InstanceHandle, Instance>& instanceData : m_instances)
		{
			const InstanceHandle instanceHandle = instanceData.first;
			const Instance& instance = instanceData.second;
			// Ignore instance type we dont have to update
			if (m_instancesDirty.has(instance.type))
				assetInstances[EnumToIndex(instance.type)][instance.assetID].append(instance.transform);
		}

		// Could have only one type dirty.
		// Generate draw command & fill instance buffer.
		for (InstanceType instanceType : m_instancesDirty)
		{
			if (instanceType != InstanceType::StaticMesh3D)
				continue; // Skip other types for now

			gfx::BufferHandle buffer = m_renderData[EnumToIndex(instanceType)].m_instanceBuffer;
			gfx::BufferHandle bufferStaging = m_renderData[EnumToIndex(instanceType)].m_instanceBufferStaging;
			InstanceData* data = static_cast<InstanceData*>(m_device->map(bufferStaging, gfx::BufferMap::Write));
			m_drawIndexedBuffer[EnumToIndex(instanceType)].clear();

			uint32_t instanceCount = 0;
			for (const std::pair<AssetID, Vector<mat4f>>& instancesPair : assetInstances[EnumToIndex(instanceType)])
			{
				AssetID assetID = instancesPair.first;
				const Vector<mat4f>& instances = instancesPair.second;
				ResourceHandle<StaticMesh> meshHandle = m_library->get<StaticMesh>(assetID);
				if (!meshHandle.isLoaded())
				{
					// Should not happen as meshes are registered only when set
					AKA_ASSERT(false, "Should not happen");
					continue; // Skip it
				}
				StaticMesh& mesh = meshHandle.get();
				AKA_ASSERT(mesh.getIndexFormat() == gfx::IndexFormat::UnsignedInt, "");
				uint32_t indexOffset = getGeometryBufferOffset(mesh.getIndexBufferHandle());
				uint32_t vertexOffset = getGeometryBufferOffset(mesh.getVertexBufferHandle());
				AKA_ASSERT((indexOffset % sizeof(uint32_t)) == 0, "Indices not aligned");
				AKA_ASSERT((vertexOffset % sizeof(StaticVertex)) == 0, "Vertices not aligned");
				for (const StaticMeshBatch& batch : mesh.getBatches())
				{
					AKA_ASSERT((batch.indexOffset % sizeof(uint32_t)) == 0, "Indices not aligned");
					AKA_ASSERT((batch.vertexOffset % sizeof(StaticVertex)) == 0, "Vertices not aligned");
					gfx::DrawIndexedIndirectCommand command{};
					command.indexCount = batch.indexCount;
					command.instanceCount = (uint32_t)instances.size();
					command.firstIndex = (batch.indexOffset + indexOffset) / sizeof(uint32_t);
					command.vertexOffset = (batch.vertexOffset + vertexOffset) / sizeof(StaticVertex);
					command.firstInstance = instanceCount; // offset in instance buffer.
					m_drawIndexedBuffer[EnumToIndex(instanceType)].append(command);
					for (const mat4f & instanceTransform : instances)
					{
						uint32_t batchID = (uint32_t)std::distance(m_materials.begin(), m_materials.find(batch.material.get().getMaterialHandle()));
						data[instanceCount].transform = instanceTransform;
						data[instanceCount].normal = mat4f::transpose(mat4f::inverse(instanceTransform));
						data[instanceCount].batchID = batchID;

						++instanceCount;
						AKA_ASSERT(MaxInstanceCount >= instanceCount, "Too many instances, need resize buffer");
					}
				}
			}
			m_device->unmap(bufferStaging);
			gfx::ScopedCmdMarker marker(cmd, String::format("PrepareBuffers_%s", getInstanceTypeName(instanceType)).cstr());
			cmd->transition(buffer, gfx::ResourceAccessType::Resource, gfx::ResourceAccessType::CopyDST);
			cmd->copy(bufferStaging, buffer);
			cmd->transition(buffer, gfx::ResourceAccessType::CopyDST, gfx::ResourceAccessType::Resource);
		}
		m_instancesDirty.clear();
	}
	
	if (m_materialDirty)
	{
		MaterialData* data = static_cast<MaterialData*>(m_device->map(m_materialStagingBuffer, gfx::BufferMap::Write));
		uint32_t materialID = 0;
		for (auto& material : m_materials)
		{
			data[materialID] = material.second.data;
			materialID++;
		}

		m_device->unmap(m_materialStagingBuffer);
		cmd->transition(m_materialBuffer, gfx::ResourceAccessType::Resource, gfx::ResourceAccessType::CopyDST);
		cmd->copy(m_materialStagingBuffer, m_materialBuffer);
		cmd->transition(m_materialBuffer, gfx::ResourceAccessType::CopyDST, gfx::ResourceAccessType::Resource);
		m_materialDirty = false;
	}

	// Draw !
	gfx::FramebufferHandle fb = m_device->get(m_backbuffer, frame);
	cmd->beginRenderPass(m_backbufferRenderPass, fb, gfx::ClearState{ gfx::ClearMask::All, {0.f, 1.f, 0.f, 1.f}, 1.f, 0 });
	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		gfx::ScopedCmdMarker marker(cmd, String::format("Render_%s", getInstanceTypeName(instanceType)).cstr());

		const InstanceRenderData& data = m_renderData[EnumToIndex(instanceType)];
		cmd->bindPipeline(data.m_pipeline);
		cmd->bindDescriptorSet(0, m_viewDescriptorSet[0]);
		cmd->bindDescriptorSet(1, m_materialSet);
		cmd->bindDescriptorSet(2, m_bindlessDescriptorSet);
		cmd->bindVertexBuffer(0, m_geometryVertexBuffer, 0);
		cmd->bindVertexBuffer(1, data.m_instanceBuffer, 0);
		cmd->bindIndexBuffer(m_geometryIndexBuffer, gfx::IndexFormat::UnsignedInt, 0);
		// TODO upload command on GPU & use indirect.
		for (gfx::DrawIndexedIndirectCommand& batch : m_drawIndexedBuffer[EnumToIndex(instanceType)])
		{
			cmd->drawIndexed(batch.indexCount, batch.firstIndex, batch.vertexOffset, batch.instanceCount, batch.firstInstance);
		}
	}
	cmd->endRenderPass();
}
void Renderer::resize(uint32_t width, uint32_t height)
{
	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		InstanceRenderData& data = m_renderData[EnumToIndex(instanceType)];
		data.m_width = width;
		data.m_height = height;
	}
	destroyRenderPass();
	createRenderPass();
}

void Renderer::onReceive(const ShaderReloadedEvent& event)
{
	getDevice()->wait();
	destroyRenderPass();
	createRenderPass();
}

MaterialHandle Renderer::createMaterial()
{
	m_materialDirty = true;

	size_t hash = hash::fnv(&materialSeed, sizeof(uint32_t));
	materialSeed++;
	MaterialHandle materialHandle = static_cast<MaterialHandle>(hash);

	uint32_t materialID = (uint32_t)m_materials.size();
	RendererMaterial material;
	material.data = MaterialData{};

	AKA_ASSERT(m_materials.find(materialHandle) == m_materials.end(), "Hash collision");

	m_materials.insert(std::make_pair(materialHandle, RendererMaterial()));
	return materialHandle;
}

void Renderer::updateMaterial(MaterialHandle handle, const color4f& color, TextureID albedo, TextureID normal)
{
	auto it = m_materials.find(handle);
	it->second.data.albedoID = EnumToValue(albedo);
	it->second.data.normalID = EnumToValue(normal);
	Memory::copy(it->second.data.color, color.data, sizeof(float) * 4);
}

void Renderer::destroyMaterial(MaterialHandle handle)
{
	m_materials.erase(handle);
}

TextureID Renderer::allocateTextureID(gfx::TextureHandle texture)
{
	AKA_ASSERT(getDevice()->get(texture)->type == gfx::TextureType::Texture2D, "");
	TextureID textureID = TextureID::Invalid;
	if (m_availableTexureID.size() > 0)
	{
		auto it = m_availableTexureID.begin();
		textureID = *it;
		m_availableTexureID.erase(it);
	}
	else
	{
		textureID = m_nextTextureID;
		m_nextTextureID = static_cast<TextureID>(EnumToValue(m_nextTextureID) + 1);
	}
	gfx::DescriptorUpdate update = gfx::DescriptorUpdate::sampledTexture2D(0, EnumToValue(textureID), texture, getSampler(SamplerType::Bilinear));
	m_device->update(m_bindlessDescriptorSet, &update, 1);
	return textureID;
}

gfx::SamplerHandle Renderer::getSampler(SamplerType type)
{
	return m_defaultSamplers[EnumToIndex(type)];
}

};


