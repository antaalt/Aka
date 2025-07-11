#include <Aka/Renderer/Renderer.hpp>

#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Core/Application.h>
#include <Aka/Core/Bitfield.h>

#include <Aka/Renderer/InstanceRenderer/SkeletalMeshInstanceRenderer.hpp>
#include <Aka/Renderer/InstanceRenderer/StaticMeshInstanceRenderer.hpp>

namespace aka {

// TODO dynamic count.

const char* getInstanceTypeName(InstanceType type)
{
	static const char* s_instanceTypeName[] = {
		"StaticMesh3D",
		"SkeletalMesh3D",
		"Sprite2D",
		"Text2D",
		"Text3D"
	};
	static_assert(countof(s_instanceTypeName) == EnumCount<InstanceType>());
	return s_instanceTypeName[EnumToIndex(type)];
}

Renderer::Renderer(AssetLibrary* _library) :
	m_instanceRenderer{ nullptr },
	m_device(nullptr),
	m_window(nullptr),
	m_shaders(nullptr),
	m_library(_library)
{
	for (InstanceType instanceType : EnumRange<InstanceType>())
		m_instanceRenderer[EnumToIndex(instanceType)] = nullptr;
	m_instanceRenderer[EnumToIndex(InstanceType::StaticMesh3D)] = mem::akaNew<StaticMeshInstanceRenderer>(AllocatorMemoryType::Object, AllocatorCategory::Graphic, std::ref(*this));
	m_instanceRenderer[EnumToIndex(InstanceType::SkeletalMesh3D)] = mem::akaNew<SkeletalMeshInstanceRenderer>(AllocatorMemoryType::Object, AllocatorCategory::Graphic, std::ref(*this));
}
Renderer::~Renderer()
{
	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		mem::akaDelete(m_instanceRenderer[EnumToIndex(instanceType)]);
	}
}

void Renderer::create(gfx::GraphicDevice* _device)
{
	m_window = Application::app()->window();
	m_shaders = Application::app()->program();
	m_device = _device;
	// Create instance buffer (need resize depending on count ?)
	gfx::SwapchainHandle swapchain = m_window->swapchain();
	gfx::SwapchainExtent extent = getDevice()->getSwapchainExtent(swapchain);
	m_width = extent.width;
	m_height = extent.height;

	{ // View
		m_viewDescriptorLayout.add(gfx::ShaderBindingType::UniformBuffer, gfx::ShaderMask::Vertex);
		m_viewDescriptorPool = getDevice()->createDescriptorPool("ViewDescriptorPool", m_viewDescriptorLayout, MaxViewCount * gfx::MaxFrameInFlight);
	}

	{ // Textures
		// Bindless
		m_bindlessDescriptorLayout.add(gfx::ShaderBindingType::SampledImage, gfx::ShaderMask::Fragment, gfx::ShaderBindingFlag::Bindless, MaxBindlessResources);
		// Should have max frame in flight set instead of pools.
		// Need to sync n buffers though...
		m_bindlessPool = getDevice()->createDescriptorPool("BindlessPool", m_bindlessDescriptorLayout, 1);
		m_bindlessDescriptorSet = getDevice()->allocateDescriptorSet("BindlessSet", m_bindlessDescriptorLayout, m_bindlessPool);

		// Samplers
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
	}
	{ // Material
		m_materialStagingBuffer = getDevice()->createBuffer("MaterialBuffer", gfx::BufferType::Storage, sizeof(MaterialData) * MaxMaterialCount, gfx::BufferUsage::Staging, gfx::BufferCPUAccess::ReadWrite);
		m_materialBuffer = getDevice()->createBuffer("MaterialBuffer", gfx::BufferType::Storage, sizeof(MaterialData) * MaxMaterialCount, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);

		m_materialDescriptorLayout.add(gfx::ShaderBindingType::StorageBuffer, gfx::ShaderMask::Vertex | gfx::ShaderMask::Fragment);
		m_materialPool = getDevice()->createDescriptorPool("MaterialDescriptorPool", m_materialDescriptorLayout, MaxMaterialCount);
		m_materialSet = getDevice()->allocateDescriptorSet("MaterialdescriptorSet", m_materialDescriptorLayout, m_materialPool);

		Vector<gfx::DescriptorUpdate> updates;
		updates.append(gfx::DescriptorUpdate::storageBuffer(0, 0, m_materialBuffer, 0, sizeof(MaterialData) * MaxMaterialCount));
		getDevice()->update(m_materialSet, updates.data(), updates.size());
	}
	{ // Geometry
		m_geometryVertexBuffer = getDevice()->createBuffer("GeometryVertexBuffer", gfx::BufferType::Vertex, MaxGeometryBufferSize, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);
		m_geometryIndexBuffer = getDevice()->createBuffer("GeometryIndexBuffer", gfx::BufferType::Index, MaxGeometryBufferSize, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);
		m_geometryDataBuffer = getDevice()->createBuffer("GeometryDataBuffer", gfx::BufferType::Storage, MaxGeometryBufferSize, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);
	}

	createBackbuffer();

	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		if (m_instanceRenderer[EnumToIndex(instanceType)] == nullptr)
			continue;
		m_instanceRenderer[EnumToIndex(instanceType)]->create();
	}

	m_debugDrawList.create(getDevice(), m_width, m_height);
}

void Renderer::destroy()
{
	m_debugDrawList.destroy(getDevice());
	destroyBackbuffer();
	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		if (m_instanceRenderer[EnumToIndex(instanceType)] == nullptr)
			continue;
		m_instanceRenderer[EnumToIndex(instanceType)]->destroy();
	}
	{ // Material
		m_device->destroy(m_materialStagingBuffer);
		m_device->destroy(m_materialBuffer);
		m_device->free(m_materialSet);
		m_device->destroy(m_materialPool);
	}
	{ // Textures
		for (SamplerType samplerType : EnumRange<SamplerType>())
		{
			m_device->destroy(m_defaultSamplers[EnumToIndex(samplerType)]);
		}
		m_device->free(m_bindlessDescriptorSet);
		m_device->destroy(m_bindlessPool);
	}
	{ // Geometry
		m_device->destroy(m_geometryDataBuffer);
		m_device->destroy(m_geometryVertexBuffer);
		m_device->destroy(m_geometryIndexBuffer);
	}
	{ // View
		for (auto& view : m_views)
		{
			for (uint32_t iFrame = 0; iFrame < gfx::MaxFrameInFlight; iFrame++)
			{
				m_device->free(view.second.descriptor[iFrame]);
				m_device->destroy(view.second.buffer[iFrame]);
			}
		}
		m_device->destroy(m_viewDescriptorPool);
	}
}

InstanceType getInstanceTypeFromAssetType(AssetType assetType)
{
	switch (assetType)
	{
	case AssetType::StaticMesh:
		return InstanceType::StaticMesh3D;
	case AssetType::SkeletalMesh:
		return InstanceType::SkeletalMesh3D;
	default:
		return InstanceType::Unknown;
	}
}

InstanceHandle Renderer::createStaticMeshInstance(AssetID assetID)
{
	AKA_ASSERT(getInstanceTypeFromAssetType(getLibrary()->getAssetInfo(assetID).type) == InstanceType::StaticMesh3D, "Invalid type");
	return m_instanceRenderer[EnumToIndex(InstanceType::StaticMesh3D)]->createInstance(assetID);
}

void Renderer::updateStaticMeshInstanceTransform(InstanceHandle instanceHandle, const mat4f& transform)
{
	m_instanceRenderer[EnumToIndex(InstanceType::StaticMesh3D)]->updateInstanceTransform(instanceHandle, transform);
}

void Renderer::destroyStaticMeshInstance(InstanceHandle instanceHandle)
{
	m_instanceRenderer[EnumToIndex(InstanceType::StaticMesh3D)]->destroyInstance(instanceHandle);
}

InstanceHandle Renderer::createSkeletalMeshInstance(AssetID assetID)
{
	AKA_ASSERT(getInstanceTypeFromAssetType(getLibrary()->getAssetInfo(assetID).type) == InstanceType::SkeletalMesh3D, "Invalid type");
	return m_instanceRenderer[EnumToIndex(InstanceType::SkeletalMesh3D)]->createInstance(assetID);
}

void Renderer::updateSkeletalMeshInstanceTransform(InstanceHandle instanceHandle, const mat4f& transform)
{
	m_instanceRenderer[EnumToIndex(InstanceType::SkeletalMesh3D)]->updateInstanceTransform(instanceHandle, transform);
}

void Renderer::destroySkeletalMeshInstance(InstanceHandle instanceHandle)
{
	m_instanceRenderer[EnumToIndex(InstanceType::SkeletalMesh3D)]->destroyInstance(instanceHandle);
}

void Renderer::updateSkeletalMeshBoneInstance(InstanceHandle instance, uint32_t index, const mat4f& transform)
{
	SkeletalMeshInstanceRenderer* renderer = static_cast<SkeletalMeshInstanceRenderer*>(m_instanceRenderer[EnumToIndex(InstanceType::SkeletalMesh3D)]);
	renderer->updateBoneInstanceTransform(instance, index, transform);
}

ViewHandle Renderer::createView(ViewType viewType)
{
	size_t hash = hash::fnv(&viewType, sizeof(ViewType));
	hash::combine(hash, m_viewSeed++);
	ViewHandle handle = static_cast<ViewHandle>(hash);
	View view;
	view.type = viewType;
	view.data = ViewData{};

	AKA_ASSERT(m_views.find(handle) == m_views.end(), "Hash collision");


	gfx::ShaderBindingState bindings{};
	bindings.add(gfx::ShaderBindingType::UniformBuffer, gfx::ShaderMask::Vertex);
	for (uint32_t iFrame = 0; iFrame < gfx::MaxFrameInFlight; iFrame++)
	{
		view.buffer[iFrame] = getDevice()->createBuffer("ViewBuffer", gfx::BufferType::Uniform, sizeof(ViewData), gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);;
		view.descriptor[iFrame] = getDevice()->allocateDescriptorSet("ViewDescSet", bindings, m_viewDescriptorPool);
		Vector<gfx::DescriptorUpdate> desc;
		desc.append(gfx::DescriptorUpdate::uniformBuffer(0, 0, view.buffer[iFrame], 0, sizeof(ViewData)));
		m_device->update(view.descriptor[iFrame], desc.data(), desc.size());

		m_viewDirty[iFrame] = true;
	}
	
	m_views.insert(std::make_pair(handle, view));

	return handle;
}

void Renderer::updateView(ViewHandle handle, const mat4f& view, const mat4f& projection)
{
	m_views[handle].data.projection = projection;
	m_views[handle].data.view = view;

	for (uint32_t iFrame = 0; iFrame < gfx::MaxFrameInFlight; iFrame++)
	{
		m_viewDirty[iFrame] = true;
	}
}

void Renderer::destroyView(ViewHandle handle)
{
	getDevice()->wait(); // Wait as buffer might be used by command buffer
	// Dirty but will be enough for now. View could use push constant instead of dedicated buffer. Data is per frame and stable enough
	View& view = m_views[handle];
	for (uint32_t iFrame = 0; iFrame < gfx::MaxFrameInFlight; iFrame++)
	{
		// use push constant for views ?
		getDevice()->destroy(view.buffer[iFrame]);
		getDevice()->free(view.descriptor[iFrame]);
		m_viewDirty[iFrame] = true;
	}
	m_views.erase(handle);
}


// TODO better allocator.

// Align to non power of 2
size_t align(size_t address, size_t alignment)
{
	if (alignment == 0)
		return address; // no alignement required
	size_t overflow = address % alignment;
	if (overflow == 0)
		return address; // already aligned

	return address + alignment - overflow;
}

GeometryBufferHandle Renderer::allocateGeometryVertex(void* data, size_t size, size_t alignement)
{
	uint32_t uSize = static_cast<uint32_t>(size);
	// Align allocation.
	m_geometryVertexBufferAllocOffset = align(m_geometryVertexBufferAllocOffset, alignement);

	AKA_ASSERT(size + m_geometryVertexBufferAllocOffset <= MaxGeometryBufferSize, "MaxGeometryBufferSize reached");
	uint32_t idBitmask = (1U << 31); // Mark it as vertex
	GeometryBufferHandle handle = static_cast<GeometryBufferHandle>(m_geometryVertexBufferAllocOffset | idBitmask);
	getDevice()->upload(m_geometryVertexBuffer, data, m_geometryVertexBufferAllocOffset, uSize);
	m_geometryVertexBufferAllocOffset += size;
	return handle;
}
GeometryBufferHandle Renderer::allocateGeometryIndex(void* data, size_t size, size_t alignement)
{
	uint32_t uSize = static_cast<uint32_t>(size);
	// Align allocation.
	m_geometryIndexBufferAllocOffset = align(m_geometryIndexBufferAllocOffset, alignement);

	AKA_ASSERT(size + m_geometryIndexBufferAllocOffset <= MaxGeometryBufferSize, "MaxGeometryBufferSize reached");
	uint32_t idBitmask = 0;
	GeometryBufferHandle handle = static_cast<GeometryBufferHandle>(m_geometryIndexBufferAllocOffset | idBitmask);
	getDevice()->upload(m_geometryIndexBuffer, data, m_geometryIndexBufferAllocOffset, uSize);
	m_geometryIndexBufferAllocOffset += size;
	return handle;
}
GeometryBufferHandle Renderer::allocateGeometryData(void* data, size_t size, size_t alignement)
{
	uint32_t uSize = static_cast<uint32_t>(size);
	// Align allocation.
	m_geometryDataBufferAllocOffset = align(m_geometryDataBufferAllocOffset, alignement);

	AKA_ASSERT(size + m_geometryDataBufferAllocOffset <= MaxGeometryBufferSize, "MaxGeometryBufferSize reached");
	uint32_t idBitmask = (1U << 30);
	GeometryBufferHandle handle = static_cast<GeometryBufferHandle>(m_geometryDataBufferAllocOffset | idBitmask);
	getDevice()->upload(m_geometryDataBuffer, data, m_geometryDataBufferAllocOffset, uSize);
	m_geometryDataBufferAllocOffset += size;
	return handle;
}

void Renderer::update(GeometryBufferHandle handle, const void* data, size_t size, size_t offset)
{
	gfx::BufferHandle buffer = getGeometryBuffer(handle);
	uint32_t offsetInBuffer = getGeometryBufferOffset(handle);
	getDevice()->upload(buffer, data, offsetInBuffer + (uint32_t)offset, (uint32_t)size);
}

void Renderer::deallocate(GeometryBufferHandle handle)
{
	// Nothing, linear allocator for now
}

gfx::BufferHandle Renderer::getGeometryBuffer(GeometryBufferHandle handle)
{
	if ((static_cast<uint32_t>(handle) >> 31) & 0x1)
		return m_geometryVertexBuffer;
	if ((static_cast<uint32_t>(handle) >> 30) & 0x1)
		return m_geometryDataBuffer;
	return m_geometryIndexBuffer;
}

uint32_t Renderer::getGeometryBufferOffset(GeometryBufferHandle handle)
{
	return static_cast<uint32_t>(handle) & bitmask<uint32_t>(30); // Remove id bit
}

void Renderer::render(gfx::FrameHandle frame)
{
	gfx::CommandList* cmd = m_device->getGraphicCommandList(frame);
	gfx::FrameIndex frameIndex = m_device->getFrameIndex(frame);

	if (m_bindlessTextureUpdates.size() > 0)
	{
		m_device->update(m_bindlessDescriptorSet, m_bindlessTextureUpdates.data(), m_bindlessTextureUpdates.size());
		m_bindlessTextureUpdates.clear();
	}

	if (m_materialDirty)
	{
		MaterialData* data = static_cast<MaterialData*>(m_device->map(m_materialStagingBuffer, gfx::BufferMap::Write));
		Memory::copy(data, m_materials.data(), m_materials.size() * sizeof(MaterialData));
		gfx::ScopedCmdMarker marker(*cmd, "UpdateMaterialData");
		m_device->unmap(m_materialStagingBuffer);
		cmd->transition(m_materialBuffer, gfx::ResourceAccessType::Resource, gfx::ResourceAccessType::CopyDST);
		cmd->copy(m_materialStagingBuffer, m_materialBuffer);
		cmd->transition(m_materialBuffer, gfx::ResourceAccessType::CopyDST, gfx::ResourceAccessType::Resource);
		m_materialDirty = false;
	}

	gfx::SwapchainHandle swapchain = m_window->swapchain();
	for (const std::pair<ViewHandle, View>& viewPair : m_views)
	{
		const ViewHandle viewHandle = viewPair.first;
		const View& view = viewPair.second;

		// TODO: framebuffer should be per view.
		gfx::FramebufferHandle fb = getDevice()->get(m_backbuffer, swapchain, frame);
		gfx::ClearState clearState{};
		clearState.setColor(0, 0.f, 1.f, 0.f, 1.f);
		clearState.setDepthStencil(1.f, 0);
		cmd->executeRenderPass(m_backbufferRenderPass, fb, clearState, [=](gfx::RenderPassCommandList& cmd) {
			AKA_ASSERT(view.type == ViewType::Color, "Only main view supported for now.");

			if (m_viewDirty[frameIndex.value()]) // TODO: one dirty per view.
			{
				ViewData ubo;
				ubo.view = view.data.view;
				ubo.projection = view.data.projection;
				m_device->upload(view.buffer[frameIndex.value()], &ubo, 0, sizeof(ViewData));
				m_viewDirty[frameIndex.value()] = false;
			}

			// TODO should prepare be per view ?
			for (InstanceType instanceType : EnumRange<InstanceType>())
			{
				if (m_instanceRenderer[EnumToIndex(instanceType)] == nullptr)
					continue;
				m_instanceRenderer[EnumToIndex(instanceType)]->prepare(view, frame);
			}
			//if (view.main)
			m_debugDrawList.prepare(frame, getDevice());
			for (InstanceType instanceType : EnumRange<InstanceType>())
			{
				if (m_instanceRenderer[EnumToIndex(instanceType)] == nullptr)
					continue;
				m_instanceRenderer[EnumToIndex(instanceType)]->render(view, frame, cmd);
			}
			//if (view.main)
			m_debugDrawList.render(getDevice(), frame, view.data.view, view.data.projection, cmd);
		});
		break; // For now only one main view.
	}
	m_debugDrawList.clear();
}
void Renderer::resize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;
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

	AKA_ASSERT(m_materialIndex.find(materialHandle) == m_materialIndex.end(), "Hash collision");

	const uint32_t materialIndex = (uint32_t)m_materials.size();
	m_materials.append(MaterialData{});
	m_materialIndex.insert(std::make_pair(materialHandle, materialIndex));
	return materialHandle;
}

void Renderer::updateMaterial(MaterialHandle handle, const color4f& color, TextureID albedo, TextureID normal)
{
	auto it = m_materialIndex.find(handle);
	const uint32_t offset = it->second;
	m_materials[offset].albedoID = EnumToValue(albedo);
	m_materials[offset].normalID = EnumToValue(normal);
	Memory::copy(m_materials[offset].color, color.data, sizeof(float) * 4);
}

void Renderer::destroyMaterial(MaterialHandle handle)
{
	m_materialIndex.erase(handle);
	// TODO mark empty slot
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
	m_bindlessTextureUpdates.append(update);
	return textureID;
}

gfx::SamplerHandle Renderer::getSampler(SamplerType type)
{
	return m_defaultSamplers[EnumToIndex(type)];
}

uint32_t Renderer::getMaterialIndex(MaterialHandle handle)
{
	return m_materialIndex.find(handle)->second;
}
void Renderer::createBackbuffer()
{
	gfx::SwapchainHandle swapchain = m_window->swapchain();
	// TODO: create depth
	m_backbufferRenderPassState = {};
	m_backbufferRenderPassState.addColor(gfx::TextureFormat::Swapchain, gfx::AttachmentLoadOp::Clear, gfx::AttachmentStoreOp::Store, gfx::ResourceAccessType::Undefined, gfx::ResourceAccessType::Present);
	m_backbufferRenderPassState.setDepth(gfx::TextureFormat::Depth24Stencil8, gfx::AttachmentLoadOp::Clear, gfx::AttachmentStoreOp::Store, gfx::ResourceAccessType::Attachment, gfx::ResourceAccessType::Attachment);

	m_depth = getDevice()->createTexture("RendererDepth", m_width, m_height, 1, gfx::TextureType::Texture2D, 1, 1, gfx::TextureFormat::Depth24Stencil8, gfx::TextureUsage::RenderTarget);
	gfx::Attachment depthAttachment;
	depthAttachment.texture = m_depth;
	depthAttachment.flag = gfx::AttachmentFlag::BackbufferAutoResize;
	
	m_backbufferRenderPass = getDevice()->createRenderPass("BackbufferPassHandle", m_backbufferRenderPassState);
	m_backbuffer = getDevice()->createBackbuffer("Backbuffer", swapchain, m_backbufferRenderPass, nullptr, 0, &depthAttachment);
}
void Renderer::destroyBackbuffer()
{
	gfx::SwapchainHandle swapchain = m_window->swapchain();
	m_device->destroy(swapchain, m_backbuffer);
	m_device->destroy(m_backbufferRenderPass);
	m_device->destroy(m_depth);
}

};


