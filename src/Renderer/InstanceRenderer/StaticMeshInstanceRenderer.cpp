#include <Aka/Renderer/InstanceRenderer/StaticMeshInstanceRenderer.hpp>

#include <Aka/Renderer/Renderer.hpp>
#include <Aka/Resource/Resource/StaticMesh.hpp>
#include <Aka/OS/OS.h>
#include <Aka/Core/Application.h>

namespace aka {

static const uint32_t MaxInstanceCount = 500;
static const uint32_t MaxAssetCount = 500;
static const uint32_t MaxBatchCount = 500;

StaticMeshInstanceRenderer::StaticMeshInstanceRenderer(Renderer& _renderer) :
	InstanceRenderer(_renderer)
{
}

void StaticMeshInstanceRenderer::create()
{
	// These buffers are resized dynamically or preallocated ?
	for (uint32_t i = 0; i < gfx::MaxFrameInFlight; i++)
		m_instanceBuffer[i] = getDevice()->createBuffer("InstanceBuffer", gfx::BufferType::Vertex, sizeof(gpu::StaticMeshInstanceData) * MaxInstanceCount, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);
	m_instanceBufferStaging = getDevice()->createBuffer("InstanceBufferStaging", gfx::BufferType::Vertex, sizeof(gpu::StaticMeshInstanceData) * MaxInstanceCount, gfx::BufferUsage::Staging, gfx::BufferCPUAccess::ReadWrite);
	m_assetBuffer = getDevice()->createBuffer("AssetBuffer", gfx::BufferType::Storage, sizeof(gpu::StaticMeshAssetData) * MaxAssetCount, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);
	m_batchBuffer = getDevice()->createBuffer("BatchBuffer", gfx::BufferType::Storage, sizeof(gpu::StaticMeshBatchData) * MaxBatchCount, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);

	gfx::ShaderBindingState modelBindings{};
	modelBindings.add(gfx::ShaderBindingType::StorageBuffer, gfx::ShaderMask::Vertex);
	modelBindings.add(gfx::ShaderBindingType::StorageBuffer, gfx::ShaderMask::Vertex);
	m_modelDescriptorPool = getDevice()->createDescriptorPool("ModelDescriptorPool", modelBindings, MaxMaterialCount);
	m_modelDescriptorSet = getDevice()->allocateDescriptorSet("ModelDescriptorSet", modelBindings, m_modelDescriptorPool);

	m_layout.addSet(getRenderer().getViewDescriptorSetLayout());
	m_layout.addSet(getRenderer().getMaterialDescriptorSetLayout());
	m_layout.addSet(getRenderer().getBindlessDescriptorSetLayout());
	m_layout.addSet(modelBindings);

	Vector<gfx::DescriptorUpdate> updates;
	updates.append(gfx::DescriptorUpdate::storageBuffer(0, 0, m_assetBuffer, 0, sizeof(gpu::StaticMeshAssetData) * MaxAssetCount));
	updates.append(gfx::DescriptorUpdate::storageBuffer(1, 0, m_batchBuffer, 0, sizeof(gpu::StaticMeshBatchData) * MaxBatchCount));
	getDevice()->update(m_modelDescriptorSet, updates.data(), updates.size());

	createPipeline();
}
void StaticMeshInstanceRenderer::destroy()
{
	destroyPipeline();
	for (uint32_t i = 0; i < gfx::MaxFrameInFlight; i++)
		getDevice()->destroy(m_instanceBuffer[i]);
	getDevice()->destroy(m_instanceBufferStaging);
	getDevice()->destroy(m_assetBuffer);
	getDevice()->destroy(m_batchBuffer);
	getDevice()->free(m_modelDescriptorSet);
	getDevice()->destroy(m_modelDescriptorPool);
}

void StaticMeshInstanceRenderer::createPipeline()
{
	ShaderRegistry* registry = Application::app()->program();
	const aka::AssetPath ShaderVertexPath = AssetPath("shaders/renderer/shader.vert", AssetPathType::Common);
	const aka::AssetPath ShaderFragmentPath = AssetPath("shaders/renderer/shader.frag", AssetPathType::Common);

	const aka::ShaderKey ShaderVertex = aka::ShaderKey().setPath(ShaderVertexPath).setType(aka::ShaderType::Vertex);
	const aka::ShaderKey ShaderFragment = aka::ShaderKey().setPath(ShaderFragmentPath).setType(aka::ShaderType::Fragment);

	m_programKey = aka::ProgramKey();
	m_programKey.add(ShaderVertex).add(ShaderFragment);
	registry->add(m_programKey, getDevice());

	gfx::RenderPassState state{};
	state.addColor(gfx::TextureFormat::Swapchain, gfx::AttachmentLoadOp::Load, gfx::AttachmentStoreOp::Store, gfx::ResourceAccessType::Present, gfx::ResourceAccessType::Present);
	state.setDepth(gfx::TextureFormat::Depth24Stencil8, gfx::AttachmentLoadOp::Clear, gfx::AttachmentStoreOp::Store, gfx::ResourceAccessType::Attachment, gfx::ResourceAccessType::Attachment);

	// TODO: share depth & render pass between renderer
	uint32_t width = 0;
	uint32_t height = 0;
	getDevice()->getBackbufferSize(width, height);
	m_depth = getDevice()->createTexture("StaticDepth", width, height, 1, gfx::TextureType::Texture2D, 1, 1, gfx::TextureFormat::Depth24Stencil8, gfx::TextureUsage::RenderTarget);
	m_backbufferRenderPass = getDevice()->createRenderPass("BackbufferPassHandle", state);
	gfx::Attachment depthAttachment;
	depthAttachment.texture = m_depth;
	depthAttachment.flag = gfx::AttachmentFlag::BackbufferAutoResize;
	m_backbuffer = getDevice()->createBackbuffer("Backbuffer", m_backbufferRenderPass, nullptr, 0, &depthAttachment);

	gfx::ProgramHandle programHandle = registry->get(m_programKey);
	// Create pipeline
	m_pipeline = getDevice()->createGraphicPipeline(
		"Graphic pipeline",
		programHandle,
		gfx::PrimitiveType::Triangles,
		m_layout,
		getDevice()->get(m_backbufferRenderPass)->state,
		gfx::VertexState{}.add(StaticVertex::getState()).add(StaticMeshInstance::getState()),
		gfx::ViewportStateBackbuffer,
		gfx::DepthStateLessEqual,
		gfx::StencilStateDefault,
		gfx::CullStateDefault,
		gfx::BlendStateDefault,
		gfx::FillStateFill
	);
}

void StaticMeshInstanceRenderer::destroyPipeline()
{
	getDevice()->destroy(m_pipeline);
	getDevice()->destroy(m_backbuffer);
	getDevice()->destroy(m_backbufferRenderPass);
	getDevice()->destroy(m_depth);
}

void StaticMeshInstanceRenderer::prepare(gfx::FrameHandle frame)
{
	if (m_instanceDatas.size() == 0)
		return;
	gfx::CommandList* cmd = getDevice()->getGraphicCommandList(frame);
	gfx::FrameIndex frameIndex = getDevice()->getFrameIndex(frame);

	// TODO: should run this for each views.
	// TODO: should use a compute shader for creating indirect buffers
	if (m_dirty[frameIndex.value()]) // if a transform changed or a material
	{
		// Generate draw command & fill instance buffer.
		m_drawIndexedBuffer.clear();

		uint32_t instanceInOffset = 0;
		uint32_t instanceOutOffset = 0;
		uint32_t instanceCount = 0;
		uint32_t instanceIndex = 0;
		AssetID currentAssetID = m_instanceDatas[0].getAssetID();
		gpu::StaticMeshInstanceData* data = static_cast<gpu::StaticMeshInstanceData*>(getDevice()->map(m_instanceBufferStaging, gfx::BufferMap::Write));
		for (const StaticMeshInstance& instance : m_instanceDatas)
		{
			// Fill draw call
			if (instance.getAssetID() != currentAssetID)
			{
				const gpu::StaticMeshAssetData& assetData = m_instanceAssetDatas[m_assetIndex[currentAssetID]];
				for (uint32_t iBatch = 0; iBatch < assetData.batchCount; iBatch++)
				{
					const gpu::StaticMeshBatchData& batchData = m_instanceBatchDatas[assetData.batchOffset + iBatch];

					gfx::DrawIndexedIndirectCommand command{};
					command.indexCount = batchData.indexCount;
					command.instanceCount = instanceCount;
					command.firstIndex = batchData.indexOffset;
					command.vertexOffset = batchData.vertexOffset;
					command.firstInstance = instanceOutOffset;
					m_drawIndexedBuffer.append(command);

					// Fill instance data
					for (uint32_t iInstance = 0; iInstance < instanceCount; iInstance++)
					{
						gpu::StaticMeshInstanceData& instanceData = data[instanceOutOffset + instanceCount * iBatch + iInstance];
						instanceData.transform = m_instanceDatas[instanceInOffset + iInstance].getTransform();
						instanceData.normal = mat4f::transpose(mat4f::inverse(instanceData.transform));;
						instanceData.batchIndex = iBatch + assetData.batchOffset;
					}
				}
				currentAssetID = instance.getAssetID();
				instanceInOffset += instanceCount;
				instanceOutOffset += instanceCount * assetData.batchCount;
				instanceCount = 0;
			}
			instanceCount++;
		}
		if (instanceCount > 0)
		{
			const gpu::StaticMeshAssetData& assetData = m_instanceAssetDatas[m_assetIndex[currentAssetID]];
			for (uint32_t iBatch = 0; iBatch < assetData.batchCount; iBatch++)
			{
				const gpu::StaticMeshBatchData& batchData = m_instanceBatchDatas[assetData.batchOffset + iBatch];

				gfx::DrawIndexedIndirectCommand command{};
				command.indexCount = batchData.indexCount;
				command.instanceCount = instanceCount;
				command.firstIndex = batchData.indexOffset;
				command.vertexOffset = batchData.vertexOffset;
				command.firstInstance = instanceOutOffset;
				m_drawIndexedBuffer.append(command);

				// Fill instance data
				for (uint32_t iInstance = 0; iInstance < instanceCount; iInstance++)
				{
					gpu::StaticMeshInstanceData& instanceData = data[instanceOutOffset + instanceCount * iBatch + iInstance];
					instanceData.transform = m_instanceDatas[instanceInOffset + iInstance].getTransform();
					instanceData.normal = mat4f::transpose(mat4f::inverse(instanceData.transform));;
					instanceData.batchIndex = iBatch + assetData.batchOffset;
				}
			}
		}
		getDevice()->unmap(m_instanceBufferStaging);

		// Upload now as we have frame in flight
		gfx::ScopedCmdMarker marker(cmd, "PrepareStaticMeshInstanceBuffers");
		getDevice()->copy(m_instanceBufferStaging, m_instanceBuffer[frameIndex.value()]);

		// TODO should not upload them everytime, frame in flight.
		getDevice()->upload(m_assetBuffer, m_instanceAssetDatas.data(), 0, (uint32_t)(m_instanceAssetDatas.size() * sizeof(gpu::StaticMeshAssetData)));
		getDevice()->upload(m_batchBuffer, m_instanceBatchDatas.data(), 0, (uint32_t)(m_instanceBatchDatas.size() * sizeof(gpu::StaticMeshBatchData)));
		
		m_dirty[frameIndex.value()] = false;
	}
}

void StaticMeshInstanceRenderer::render(const View& view, gfx::FrameHandle frame)
{
	gfx::CommandList* cmd = getDevice()->getGraphicCommandList(frame);
	gfx::FrameIndex frameIndex = getDevice()->getFrameIndex(frame);
	// TODO each view should have somewhere its target written.
	gfx::FramebufferHandle fb = getDevice()->get(m_backbuffer, frame);
	cmd->beginRenderPass(m_backbufferRenderPass, fb);
	if (m_drawIndexedBuffer.size() > 0)
	{
		gfx::ScopedCmdMarker marker(cmd, "RenderStaticMeshInstances");

		cmd->bindPipeline(m_pipeline);
		cmd->bindDescriptorSet(0, view.descriptor[frameIndex.value()]);
		cmd->bindDescriptorSet(1, getRenderer().getMaterialDescriptorSet());
		cmd->bindDescriptorSet(2, getRenderer().getBindlessDescriptorSet());
		cmd->bindDescriptorSet(3, m_modelDescriptorSet);
		// TODO use bindless
		cmd->bindVertexBuffer(0, getRenderer().getVertexGeometryBuffer(), 0);
		cmd->bindVertexBuffer(1, m_instanceBuffer[frameIndex.value()], 0);
		cmd->bindIndexBuffer(getRenderer().getIndexGeometryBuffer(), gfx::IndexFormat::UnsignedInt, 0);
		// TODO upload command on GPU & use indirect.
		for (gfx::DrawIndexedIndirectCommand& batch : m_drawIndexedBuffer)
		{
			cmd->drawIndexed(batch.indexCount, batch.firstIndex, batch.vertexOffset, batch.instanceCount, batch.firstInstance);
		}
	}
	cmd->endRenderPass();
}

InstanceHandle StaticMeshInstanceRenderer::createInstance(AssetID assetID)
{
	using InstanceHandleType = std::underlying_type<InstanceHandle>::type;
	InstanceHandle instanceHandle = static_cast<InstanceHandle>(geometry::random<InstanceHandleType>(0, std::numeric_limits<InstanceHandleType>::max()));
	AKA_ASSERT(m_instanceIndex.find(instanceHandle) == m_instanceIndex.end(), "Hash collision");

	ResourceHandle<StaticMesh> meshHandle = getRenderer().getLibrary()->get<StaticMesh>(assetID);
	const StaticMesh& mesh = meshHandle.get();

	auto itAsset = m_assetIndex.find(assetID);
	if (itAsset == m_assetIndex.end())
	{
		const uint32_t assetIndex = (uint32_t)m_instanceAssetDatas.size();
		const uint32_t batchOffset = (uint32_t)m_instanceBatchDatas.size();

		gpu::StaticMeshAssetData assetData{};
		assetData.batchCount = mesh.getBatchCount();
		assetData.batchOffset = batchOffset;
		m_instanceAssetDatas.push_back(assetData);


		AKA_ASSERT(mesh.getIndexFormat() == gfx::IndexFormat::UnsignedInt, "");
		uint32_t indexOffset = getRenderer().getGeometryBufferOffset(mesh.getIndexBufferHandle());
		uint32_t vertexOffset = getRenderer().getGeometryBufferOffset(mesh.getVertexBufferHandle());
		AKA_ASSERT((indexOffset % sizeof(uint32_t)) == 0, "Indices not aligned");
		AKA_ASSERT((vertexOffset % sizeof(StaticVertex)) == 0, "Vertices not aligned");

		for (uint32_t i = 0; i < mesh.getBatchCount(); i++)
		{
			const StaticMeshBatch& batch = mesh.getBatch(i);
			AKA_ASSERT((batch.indexOffset % sizeof(uint32_t)) == 0, "Indices not aligned");
			AKA_ASSERT((batch.vertexOffset % sizeof(StaticVertex)) == 0, "Vertices not aligned");
			gpu::StaticMeshBatchData batchData{};
			batchData.indexOffset = (batch.indexOffset + indexOffset) / sizeof(uint32_t);
			batchData.indexCount = batch.indexCount;
			batchData.vertexOffset = (batch.vertexOffset + vertexOffset) / sizeof(StaticVertex);
			batchData.materialIndex = getRenderer().getMaterialIndex(batch.material.get().getMaterialHandle());
			batchData.min; // TODO
			batchData.max;
			m_instanceBatchDatas.push_back(batchData);
		}

		m_assetIndex.insert(std::make_pair(assetID, assetIndex));

		StaticMeshInstance instance(assetID, ViewTypeMask::All);

		uint32_t instanceOffset = (uint32_t)m_instanceDatas.size();
		m_instanceIndex.insert(std::make_pair(instanceHandle, instanceOffset));
		m_instanceDatas.push_back(instance);
	}
	else
	{
		StaticMeshInstance instance(assetID, ViewTypeMask::All);

		// Insert the instance near to other instances.
		// Find other instance using same asset to group them for instanced draw call.
		// Use reverse iterator to append at the end and copy less data that with forward.
		auto itInstance = std::find_if(m_instanceDatas.rbegin(), m_instanceDatas.rend(), [assetID](const StaticMeshInstance& data) {
			return data.getAssetID() == assetID;
		});
		if (itInstance != m_instanceDatas.rend())
		{
			const uint32_t instanceOffset = (uint32_t)std::distance(m_instanceDatas.begin(), itInstance.base()); // Dont -1 cuz we want to insert after
			// Update index of element moved in vector.
			for (auto& instance : m_instanceIndex)
			{
				if (instance.second >= instanceOffset)
					instance.second++;
			}
			m_instanceIndex.insert(std::make_pair(instanceHandle, instanceOffset));
			m_instanceDatas.insert(m_instanceDatas.begin() + instanceOffset, instance);
		}
		else
		{
			uint32_t instanceOffset = (uint32_t)m_instanceDatas.size();
			m_instanceIndex.insert(std::make_pair(instanceHandle, instanceOffset));
			m_instanceDatas.push_back(instance);
		}
	}

	for (uint32_t i =0; i< gfx::MaxFrameInFlight; i++)
		m_dirty[i] = true;

	return instanceHandle;
}
void StaticMeshInstanceRenderer::updateInstanceTransform(InstanceHandle instanceHandle, const mat4f& transform)
{
	auto it = m_instanceIndex.find(instanceHandle);
	AKA_ASSERT(it != m_instanceIndex.end(), "");

	m_instanceDatas[it->second].setTransform(transform);

	for (uint32_t i = 0; i < gfx::MaxFrameInFlight; i++)
		m_dirty[i] = true;
}
void StaticMeshInstanceRenderer::destroyInstance(InstanceHandle instanceHandle)
{
	auto it = m_instanceIndex.find(instanceHandle);
	AKA_ASSERT(it != m_instanceIndex.end(), "");

	uint32_t instanceIndex = it->second;

	m_instanceIndex.erase(it);
	for (auto& instance : m_instanceIndex)
	{
		if (instance.second > instanceIndex)
			instance.second--;
	}
	m_instanceDatas.erase(m_instanceDatas.begin() + instanceIndex);
	//StaticMeshInstance& instance = m_instanceDatas[instanceIndex];
	// TODO remove given instance, offset all instance index in m_instanceIndex.
	// Optional : check if no more instance for the used asset & clean asset

	// TODO remove assets if no more instances reference them.
	// For now, we simply keep them in buffer...
	/*uint32_t assetIndex = m_assetIndex[instance.getAssetID()];
	// TODO check if no more instance in here.
	gpu::StaticMeshAssetData& assetData = m_instanceAssetDatas[assetIndex];
	for (uint32_t i = 0; i < assetData.batchCount; i++)
	{
		gpu::StaticMeshBatchData& batchData = m_instanceBatchDatas[assetData.batchOffset + i];
	}
	// Remove batch.
	m_instanceBatchDatas.erase(m_instanceBatchDatas.begin() + assetData.batchOffset, m_instanceBatchDatas.begin() + assetData.batchOffset + assetData.batchCount);
	// We reoffset all asset following the one we have erased.
	for (uint32_t iAsset = assetIndex; iAsset < (uint32_t)m_instanceAssetDatas.size(); iAsset++)
	{
		m_instanceAssetDatas[iAsset].batchOffset -= assetData.batchCount;
	}
	m_instanceAssetDatas.erase(m_instanceAssetDatas.begin() + assetIndex);
	AKA_NOT_IMPLEMENTED;
	*/
	for (uint32_t i = 0; i < gfx::MaxFrameInFlight; i++)
		m_dirty[i] = true;
}

}