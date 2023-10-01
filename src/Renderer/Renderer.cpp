#include <Aka/Renderer/Renderer.hpp>

#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Core/Application.h>

namespace aka {

Renderer::Renderer(gfx::GraphicDevice* _device, AssetLibrary* _library) :
	m_device(_device),
	m_library(_library)
{
}
Renderer::~Renderer()
{
}

// TODO dynamic count.
static const uint32_t MaxInstanceCount = 500;
static const uint32_t MaxViewCount = 5;
static const uint32_t MaxBindlessResources = 16536;

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
	destroyRenderPass();
	m_device->destroy(m_viewBuffers);
	m_device->free(m_bindlessDescriptorSet);
	m_device->destroy(m_bindlessPool);
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

Instance* Renderer::createInstance(AssetID assetID)
{
	InstanceType type = getInstanceTypeFromAssetType(m_library->getAssetInfo(assetID).type);
	auto& it = m_assetInstances[EnumToIndex(type)][assetID];

	gfx::ShaderBindingState bindings{};
	bindings.add(gfx::ShaderBindingType::UniformBuffer, gfx::ShaderMask::Vertex);

	Instance* instance = new Instance;
	it.push_back(instance);
	instance->type = type;
	instance->assetID = assetID;
	instance->mask = ViewTypeMask::Color;
	instance->transform = mat4f::identity();
	return instance;
}


void Renderer::destroyInstance(Instance*& instance)
{
	InstanceType type = getInstanceTypeFromAssetType(m_library->getAssetInfo(instance->assetID).type);
	AKA_ASSERT(instance->type == type, "");
	std::vector<Instance*>& data = m_assetInstances[EnumToIndex(type)][instance->assetID];
	auto itFind = std::find(data.begin(), data.end(), instance);
	if (itFind != data.end())
	{
		data.erase(itFind);
		delete instance;
		instance = nullptr;
	}
	else
	{
		AKA_ASSERT(false, "trying to destroy an instance that does not exist");
	}
}


View* Renderer::createView(ViewType viewType)
{
	View* view = m_views.append(new View);
	view->type = viewType;
	view->data = ViewData{};
	//m_viewDescriptorSet.append(gfx::DescriptorSetHandle::null);
	return view;
}

void Renderer::destroyView(View*& view)
{
	auto it = std::find(m_views.begin(), m_views.end(), view);
	if (it != m_views.end())
	{
		m_views.remove(it);
	}
	delete view;
	view = nullptr;
}

void Renderer::render(gfx::Frame* frame)
{
	gfx::CommandList* cmd = m_device->getGraphicCommandList(frame);

	//std::map<AssetID, Vector<gfx::DrawIndexedIndirectCommand>> drawIndexedBuffer[EnumCount<InstanceType>()];
	// Update instance buffer (TODO handle gfx::MaxFrameInFlight)

	if (m_views.size() == 0)
		return;
	View* view = m_views[0];
	if (view == nullptr)
		return;

	ViewData ubo;
	ubo.view = view->data.view;
	ubo.projection = view->data.projection;
	m_device->upload(m_viewBuffers, &ubo, 0, sizeof(ViewData));

	static const char* s_instanceTypeName[] = {
		"StaticMesh3D",
		"Sprite2D",
		"Text2D",
		"Text3D"
	};
	static_assert(countof(s_instanceTypeName) == EnumCount<InstanceType>());
	
	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		if (instanceType != InstanceType::StaticMesh3D)
			continue; // Skip other types for now
		gfx::ScopedCmdMarker marker(cmd, String::format("PrepareBuffers_%s", s_instanceTypeName[EnumToIndex(instanceType)]).cstr());
		uint32_t instanceCount = 0;
		gfx::BufferHandle buffer = m_renderData[EnumToIndex(instanceType)].m_instanceBuffer;
		gfx::BufferHandle bufferStaging = m_renderData[EnumToIndex(instanceType)].m_instanceBufferStaging;

		InstanceData* data = static_cast<InstanceData*>(m_device->map(bufferStaging, gfx::BufferMap::Write));
		//uint32_t instanceBufferAssetOffset = 0;
		Vector<gfx::DescriptorUpdate> desc;
		for (auto& assetInstances : m_assetInstances[EnumToIndex(instanceType)])
		{
			// write offset
			const AssetID assetID = assetInstances.first;
			const std::vector<Instance*> instances = assetInstances.second;
			ResourceHandle<StaticMesh> meshHandle = m_library->get<StaticMesh>(assetID);
			if (!meshHandle.isLoaded())
				continue; // Skip it
			StaticMesh& mesh = meshHandle.get();
			for (Instance* instance : instances)
			{
				data[instanceCount].transform = instance->transform;
				data[instanceCount].normal = mat4f::transpose(mat4f::inverse(instance->transform));

				++instanceCount;
				AKA_ASSERT(MaxInstanceCount >= instanceCount, "Too many instances, need resize buffer");
			}
			for (const StaticMeshBatch& batch : mesh.getBatches())
			{
				desc.append(gfx::DescriptorUpdate::sampledTexture2D(0, (uint32_t)desc.size(), batch.m_albedo.get().getGfxHandle(), mesh.getAlbedoSampler()));
				desc.append(gfx::DescriptorUpdate::sampledTexture2D(0, (uint32_t)desc.size(), batch.m_normal.get().getGfxHandle(), mesh.getNormalSampler()));
			}
			/*for (const StaticMeshBatch& batch : mesh.getBatches())
			{
				gfx::DrawIndexedIndirectCommand command{};
				command.indexCount = batch.indexCount;
				command.instanceCount = instances.size();
				command.firstIndex = batch.indexOffset;
				command.vertexOffset = batch.vertexOffset;
				command.firstInstance = instanceBufferAssetOffset; // offset in instance buffer.
				drawIndexedBuffer[EnumToIndex(instanceType)][assetID].append(command);
				// TODO draw on GPU instead of keeping in local.
			}
			instanceBufferAssetOffset += instanceCount;*/
		}
		m_device->unmap(bufferStaging);
		cmd->transition(buffer, gfx::ResourceAccessType::Resource, gfx::ResourceAccessType::CopyDST);
		cmd->copy(bufferStaging, buffer);
		cmd->transition(buffer, gfx::ResourceAccessType::CopyDST, gfx::ResourceAccessType::Resource);

		for (auto k : m_library->getRange<Texture>())
		{
			ResourceHandle<Texture> texHandle = k.second;
			if (texHandle.isLoaded())
			{
				Texture& texture = texHandle.get();
				gfx::TextureHandle handle = texture.getGfxHandle();
			}
		}
		// Bindless
		m_device->update(m_bindlessDescriptorSet, desc.data(), desc.size());
	}
	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		if (instanceType != InstanceType::StaticMesh3D)
			continue; // Skip other types for now
		gfx::ScopedCmdMarker marker(cmd, String::format("Render_%s", s_instanceTypeName[EnumToIndex(instanceType)]).cstr());
		uint32_t instanceID = 0;
		//const auto& assetDrawCommands = drawIndexedBuffer[EnumToIndex(instanceType)];
		const InstanceRenderData& data = m_renderData[EnumToIndex(instanceType)];
		// Cant clear with this pass as it will discard previous instance type
		gfx::FramebufferHandle fb = m_device->get(m_backbuffer, frame);
		cmd->beginRenderPass(m_backbufferRenderPass, fb, gfx::ClearState{ gfx::ClearMask::All, {0.f, 1.f, 0.f, 1.f}, 1.f, 0 });
		cmd->bindPipeline(data.m_pipeline);
		uint32_t instanceCountOffset = 0;
		for (auto& assetInstances : m_assetInstances[EnumToIndex(instanceType)])
		{
			const AssetID assetID = assetInstances.first;
			const std::vector<Instance*>& instances = assetInstances.second;
			// TODO should prepare buffers before hand & do indirect.
			/*for (gfx::DrawIndexedIndirectCommand& batch : batches)
			{
				cmd->bindVertexBuffer(0, m.gfxVertexBuffer);
				cmd->bindVertexBuffer(1, data.m_instanceBuffer);
				cmd->bindIndexBuffer(m.gfxIndexBuffer, m.getIndexFormat(), 0);
			}*/

			// TODO cache this ? templating ?
			ResourceHandle<StaticMesh> mesh = m_library->get<StaticMesh>(assetID);
			if (mesh.isLoaded())
			{
				StaticMesh& m = mesh.get();
				gfx::ScopedCmdMarker marker(cmd, m.getName().cstr());
				cmd->bindVertexBuffer(0, m.getVertexBuffer());
				cmd->bindVertexBuffer(1, data.m_instanceBuffer, instanceCountOffset * sizeof(InstanceData));
				cmd->bindIndexBuffer(m.getIndexBuffer(), m.getIndexFormat(), 0);
				cmd->bindDescriptorSet(0, m_viewDescriptorSet[0]);
				cmd->bindDescriptorSet(2, m_bindlessDescriptorSet);
				for (uint32_t i = 0; i < m.getBatchCount(); i++)
				{
					const StaticMeshBatch& batch = m.getBatch(i);
					cmd->bindDescriptorSet(1, batch.gfxDescriptorSet); // Could skip this with bindless & use indirect everywhere
					cmd->drawIndexed(batch.indexCount, batch.indexOffset, batch.vertexOffset, (uint32_t)instances.size());
				}
			}
			instanceCountOffset += (uint32_t)instances.size();
		}
		cmd->endRenderPass();
	}
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

};


