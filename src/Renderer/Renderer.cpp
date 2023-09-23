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

void Renderer::create()
{
	m_backbufferRenderPass = m_device->createBackbufferRenderPass();
	m_backbuffer = m_device->createBackbuffer(m_backbufferRenderPass);
	// Create instance buffer (need resize depending on count ?)
	ShaderRegistry* registry = Application::app()->program();
	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		// TODO: Should use JSON for this ? JSON that can be generated with a script reading all files in shaders folder (generating DB)
		const aka::Path ShaderVertexPath = aka::OS::cwd() + "../../../asset/shaders/shader.vert";
		const aka::Path ShaderFragmentPath = aka::OS::cwd() + "../../../asset/shaders/shader.frag";

		const aka::ShaderKey ShaderVertex = aka::ShaderKey().setPath(ShaderVertexPath).setType(aka::ShaderType::Vertex);
		const aka::ShaderKey ShaderFragment = aka::ShaderKey().setPath(ShaderFragmentPath).setType(aka::ShaderType::Fragment);

		aka::ProgramKey programKey = aka::ProgramKey();
		programKey.add(ShaderVertex).add(ShaderFragment);

		registry->add(programKey, m_device);

		gfx::ProgramHandle program = registry->get(programKey);
		RenderData& data = m_renderData[EnumToIndex(instanceType)];
		data.m_pipeline = m_device->createGraphicPipeline(
			"Graphic pipeline",
			program,
			gfx::PrimitiveType::Triangles,
			m_device->get(m_backbufferRenderPass)->state,
			gfx::VertexState{}.add(StaticVertex::getState()),
			gfx::ViewportState{}.size(1280, 720),
			gfx::DepthStateLessEqual,
			gfx::StencilStateDefault,
			gfx::CullStateDefault,
			gfx::BlendStateDefault,
			gfx::FillStateFill
		);
		// These buffers are resized dynamically or preallocated ?
		data.m_instanceBuffer = m_device->createBuffer("InstanceBuffer", gfx::BufferType::Uniform, sizeof(InstanceData) * MaxInstanceCount, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);
		data.m_instanceBufferStaging = m_device->createBuffer("InstanceBuffer", gfx::BufferType::Uniform, sizeof(InstanceData) * MaxInstanceCount, gfx::BufferUsage::Staging, gfx::BufferCPUAccess::ReadWrite);
	}
	gfx::ShaderBindingState bindings{};
	bindings.add(gfx::ShaderBindingType::UniformBuffer, gfx::ShaderMask::Vertex, 1);
	m_viewBuffers = m_device->createBuffer("ViewBuffer", gfx::BufferType::Uniform, sizeof(ViewData) * MaxViewCount, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);
	m_viewDescriptorSet.append(m_device->createDescriptorSet("ViewDescSet", bindings));

	gfx::DescriptorSetData data{};
	data.addUniformBuffer(m_viewBuffers, 0, sizeof(ViewData));
	m_device->update(m_viewDescriptorSet[0], data);
}

void Renderer::destroy()
{
	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		RenderData& data = m_renderData[EnumToIndex(instanceType)];
		m_device->destroy(data.m_instanceBuffer);
		m_device->destroy(data.m_instanceBufferStaging);
		m_device->destroy(data.m_pipeline);
		for (auto& assetInstances : m_assetInstances[EnumToIndex(instanceType)])
		{
			for (auto& instance : assetInstances.second)
			{
				m_device->destroy(instance->descriptorSet);
			}
		}
	}
	m_device->destroy(m_backbufferRenderPass);
	m_device->destroy(m_backbuffer);
	m_device->destroy(m_viewBuffers);
	m_device->destroy(m_viewDescriptorSet[0]);
}

InstanceType getInstanceTypeFromAssetType(AssetType assetType)
{
	switch (assetType)
	{
	case aka::AssetType::StaticMesh:
		return InstanceType::StaticMesh3D;
	default:
		return InstanceType::Unknown;
	}
}

Instance* Renderer::createInstance(AssetID assetID)
{
	InstanceType type = getInstanceTypeFromAssetType(m_library->getAssetInfo(assetID).type);
	auto& it = m_assetInstances[EnumToIndex(type)][assetID];

	gfx::ShaderBindingState bindings{};
	bindings.add(gfx::ShaderBindingType::UniformBuffer, gfx::ShaderMask::Vertex, 1);

	Instance* instance = new Instance;
	it.append(instance);
	instance->assetID = assetID;
	instance->descriptorSet = m_device->createDescriptorSet("InstanceDescSet", bindings);
	instance->mask = ViewTypeMask::Color;
	instance->transform = mat4f::identity();;
	return instance;
}


void Renderer::destroyInstance(Instance* instance)
{
	auto& it = m_assetInstances[EnumToIndex(instance->type)][instance->assetID];
	auto itFind = std::find(it.begin(), it.end(), instance);
	if (itFind != it.end())
	{
		it.remove(itFind);
	}
	delete instance;
}


View* Renderer::createView(ViewType viewType)
{
	View* view = m_views.append(new View);
	view->type = viewType;
	view->data = ViewData{};
	//m_viewDescriptorSet.append(gfx::DescriptorSetHandle::null);
	return view;
}

void Renderer::destroyView(View* view)
{
	auto it = std::find(m_views.begin(), m_views.end(), view);
	if (it != m_views.end())
	{
		m_views.remove(it);
	}
	delete view;
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

	
	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		gfx::ScopedCmdMarker marker(cmd, "InstanceTypeSettingBuffer");
		uint32_t instanceCount = 0;
		if (instanceType != InstanceType::StaticMesh3D)
			continue; // Skip other types for now
		gfx::BufferHandle buffer = m_renderData[EnumToIndex(instanceType)].m_instanceBuffer;
		gfx::BufferHandle bufferStaging = m_renderData[EnumToIndex(instanceType)].m_instanceBufferStaging;

		InstanceData* data = static_cast<InstanceData*>(m_device->map(bufferStaging, gfx::BufferMap::Write));
		//uint32_t instanceBufferAssetOffset = 0;
		for (auto& assetInstances : m_assetInstances[EnumToIndex(instanceType)])
		{
			// write offset
			const AssetID assetID = assetInstances.first;
			const Vector<Instance*> instances = assetInstances.second;
			const ResourceID resourceID = m_library->getResourceID(assetID);
			ResourceHandle<StaticMesh> meshHandle = m_library->get<StaticMesh>(resourceID);
			if (!meshHandle.isLoaded())
				continue; // Skip it
			StaticMesh& mesh = meshHandle.get();
			for (Instance* instance : instances)
			{
				data[instanceCount].transform = instance->transform;
				data[instanceCount].normal = mat4f::transpose(mat4f::inverse(instance->transform));

				gfx::DescriptorSetData data;
				data.addUniformBuffer(buffer, instanceCount * sizeof(InstanceData), sizeof(InstanceData));
				getDevice()->update(instance->descriptorSet, data);

				++instanceCount;
				AKA_ASSERT(MaxInstanceCount >= instanceCount, "Too many instances, need resize buffer");
			}
			/*for (const auto& batch : mesh.batches)
			{
				gfx::DrawIndexedIndirectCommand command{};
				command.indexCount = batch.indexCount;
				command.instanceCount = instances.size();
				command.firstIndex = batch.indexOffset;
				command.vertexOffset = batch.vertexOffset;
				command.firstInstance = instanceBufferAssetOffset; // offset in instance buffer.
				drawIndexedBuffer[EnumToIndex(instanceType)][assetID].append(command);
			}
			instanceBufferAssetOffset += instanceCount;*/
		}
		m_device->unmap(bufferStaging);
		cmd->transition(buffer, gfx::ResourceAccessType::Resource, gfx::ResourceAccessType::CopyDST);
		cmd->copy(bufferStaging, buffer);
		cmd->transition(buffer, gfx::ResourceAccessType::CopyDST, gfx::ResourceAccessType::Resource);
	}

	for (InstanceType instanceType : EnumRange<InstanceType>())
	{
		gfx::ScopedCmdMarker marker(cmd, "InstanceTypeRendering");
		if (instanceType != InstanceType::StaticMesh3D)
			continue; // Skip other types for now
		uint32_t instanceID = 0;
		//const auto& assetDrawCommands = drawIndexedBuffer[EnumToIndex(instanceType)];
		const RenderData& data = m_renderData[EnumToIndex(instanceType)];
		// Cant clear with this pass as it will discard previous instance type
		gfx::FramebufferHandle fb = m_device->get(m_backbuffer, frame);
		cmd->beginRenderPass(m_backbufferRenderPass, fb, gfx::ClearState{ gfx::ClearMask::All, {0.f, 1.f, 0.f, 1.f}, 1.f, 0 });
		cmd->bindPipeline(data.m_pipeline);
		for (auto& assetInstances : m_assetInstances[EnumToIndex(instanceType)])
		{
			const AssetID assetID = assetInstances.first;
			const Vector<Instance*>& instances = assetInstances.second;
			// TODO should prepare buffers before hand & do indirect.
			/*for (gfx::DrawIndexedIndirectCommand& batch : batches)
			{
				cmd->bindVertexBuffer(0, m.gfxVertexBuffer);
				cmd->bindVertexBuffer(1, data.m_instanceBuffer);
				cmd->bindIndexBuffer(m.gfxIndexBuffer, m.getIndexFormat(), 0);
			}*/

			// TODO cache this ? templating ?
			ResourceID resourceID = m_library->getResourceID(assetID);
			ResourceHandle<StaticMesh> mesh = m_library->get<StaticMesh>(resourceID);
			if (mesh.isLoaded())
			{
				StaticMesh& m = mesh.get();
				gfx::ScopedCmdMarker marker(cmd, m.getName().cstr());
				cmd->bindVertexBuffer(0, m.gfxVertexBuffer);
				//cmd->bindVertexBuffer(1, data.m_instanceBuffer);
				cmd->bindIndexBuffer(m.gfxIndexBuffer, m.getIndexFormat(), 0);
				cmd->bindDescriptorSet(0, m_viewDescriptorSet[0]);
				// For now, do not use indexing as its not ready
				for (uint32_t iInstance = 0; iInstance < instances.size(); iInstance++)
				{
					cmd->bindDescriptorSet(1, instances[iInstance]->descriptorSet);
					for (const auto& batch : m.batches)
					{
						cmd->bindDescriptorSet(2, batch.gfxDescriptorSet);
						cmd->drawIndexed(batch.indexCount, batch.indexOffset, batch.vertexOffset, 1);
					}
				}
			}
		}
		cmd->endRenderPass();
	}
}

};


