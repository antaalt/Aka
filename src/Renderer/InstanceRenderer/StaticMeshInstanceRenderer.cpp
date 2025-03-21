#include <Aka/Renderer/InstanceRenderer/StaticMeshInstanceRenderer.hpp>

#include <Aka/Renderer/Renderer.hpp>
#include <Aka/Resource/Resource/StaticMesh.hpp>
#include <Aka/OS/OS.h>
#include <Aka/Core/Application.h>

namespace aka {

static const uint32_t MaxInstanceCount = 500;
static const uint32_t MaxAssetCount = 2000;
static const uint32_t MaxBatchCount = 2000;

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

	m_batchBufferStaging = getDevice()->createBuffer("StaticMeshBatchStagingBuffer", gfx::BufferType::Storage, (uint32_t)(10 * sizeof(gpu::StaticMeshBatchData)), gfx::BufferUsage::Staging, gfx::BufferCPUAccess::Write);
	m_assetBufferStaging = getDevice()->createBuffer("StaticMeshAssetStagingBuffer", gfx::BufferType::Storage, (uint32_t)(10 * sizeof(gpu::StaticMeshAssetData)), gfx::BufferUsage::Staging, gfx::BufferCPUAccess::Write);

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
	getDevice()->destroy(m_assetBufferStaging);
	getDevice()->destroy(m_batchBufferStaging);
}

void StaticMeshInstanceRenderer::createPipeline()
{
	ShaderRegistry* registry = Application::app()->program();
	PlatformWindow* window = Application::app()->window();
	const aka::AssetPath ShaderVertexPath = AssetPath("shaders/renderer/shader.vert", AssetPathType::Common);
	const aka::AssetPath ShaderFragmentPath = AssetPath("shaders/renderer/shader.frag", AssetPathType::Common);

	const aka::ShaderKey ShaderVertex = aka::ShaderKey().setPath(ShaderVertexPath).setType(aka::ShaderType::Vertex);
	const aka::ShaderKey ShaderFragment = aka::ShaderKey().setPath(ShaderFragmentPath).setType(aka::ShaderType::Fragment);

	m_programKey = aka::ProgramKey();
	m_programKey.add(ShaderVertex).add(ShaderFragment);
	registry->add(m_programKey, getDevice());

	gfx::ProgramHandle programHandle = registry->get(m_programKey);
	// Create pipeline
	m_pipeline = getDevice()->createGraphicPipeline(
		"Graphic pipeline",
		programHandle,
		gfx::PrimitiveType::Triangles,
		m_layout,
		getRenderer().getRenderPassState(),
		gfx::VertexState{}.add(StaticVertex::getState()).add(StaticMeshInstance::getState()),
		gfx::ViewportState{}.backbuffer(window->swapchain()),
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
}

void StaticMeshInstanceRenderer::prepare(const View& view, gfx::FrameHandle frame)
{
	PlatformWindow* window = Application::app()->window();
	if (m_instanceDatas.size() == 0 /*|| !view.main*/)
		return;
	gfx::CommandList* cmd = getDevice()->getGraphicCommandList(window->swapchain(), frame);
	gfx::FrameIndex frameIndex = getDevice()->getFrameIndex(window->swapchain(), frame);

	// TODO: should run this for each views.
	// TODO: should use a compute shader for creating indirect buffers
	if (m_dirtyInstance[frameIndex.value()] || m_dirtyAsset[frameIndex.value()]) // if a transform changed or a material
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

		// Recreate staging if required.
		uint32_t sizeAsset = (uint32_t)(m_instanceAssetDatas.size() * sizeof(gpu::StaticMeshAssetData));
		uint32_t sizeBatch = (uint32_t)(m_instanceBatchDatas.size() * sizeof(gpu::StaticMeshBatchData));
		if (getDevice()->get(m_assetBufferStaging)->size < sizeBatch)
		{
			getDevice()->destroy(m_assetBufferStaging);
			m_assetBufferStaging = getDevice()->createBuffer("StaticMeshAssetStagingBuffer", gfx::BufferType::Storage, sizeAsset, gfx::BufferUsage::Staging, gfx::BufferCPUAccess::Write);
		}
		if (getDevice()->get(m_batchBufferStaging)->size < sizeBatch)
		{
			getDevice()->destroy(m_batchBufferStaging);
			m_batchBufferStaging = getDevice()->createBuffer("StaticMeshBatchStagingBuffer", gfx::BufferType::Storage, sizeBatch, gfx::BufferUsage::Staging, gfx::BufferCPUAccess::Write);
		}
		{
			void* data = getDevice()->map(m_assetBufferStaging, gfx::BufferMap::Write);
			memcpy(data, m_instanceAssetDatas.data(), (uint32_t)(m_instanceAssetDatas.size() * sizeof(gpu::StaticMeshAssetData)));
			getDevice()->unmap(m_assetBufferStaging);
		}
		{
			void* data = getDevice()->map(m_batchBufferStaging, gfx::BufferMap::Write);
			memcpy(data, m_instanceBatchDatas.data(), (uint32_t)(m_instanceBatchDatas.size() * sizeof(gpu::StaticMeshBatchData)));
			getDevice()->unmap(m_batchBufferStaging);
		}

		// Upload now as we have frame in flight
		getDevice()->execute("PrepareStaticMeshInstanceBuffers", [=](gfx::CommandList& cmd) {
			cmd.copy(m_instanceBufferStaging, m_instanceBuffer[frameIndex.value()]);
			if (m_dirtyAsset[frameIndex.value()])
			{
				cmd.copy(m_assetBufferStaging, m_assetBuffer, 0, 0, sizeAsset);
				cmd.copy(m_batchBufferStaging, m_batchBuffer, 0, 0, sizeBatch);
			}
		}, gfx::QueueType::Copy); // Async

		m_dirtyInstance[frameIndex.value()] = false;
		m_dirtyAsset[frameIndex.value()] = false;
	}
}

void StaticMeshInstanceRenderer::render(const View& view, gfx::FrameHandle frame, gfx::RenderPassCommandList& cmd)
{
	PlatformWindow* window = Application::app()->window();
	gfx::FrameIndex frameIndex = getDevice()->getFrameIndex(window->swapchain(), frame);
	gfx::ScopedCmdMarker marker(cmd, "RenderStaticMeshInstances");
	if (m_drawIndexedBuffer.size() > 0)
	{
		cmd.bindPipeline(m_pipeline);
		cmd.bindDescriptorSet(0, view.descriptor[frameIndex.value()]);
		cmd.bindDescriptorSet(1, getRenderer().getMaterialDescriptorSet());
		cmd.bindDescriptorSet(2, getRenderer().getBindlessDescriptorSet());
		cmd.bindDescriptorSet(3, m_modelDescriptorSet);
		// TODO use bindless
		cmd.bindVertexBuffer(0, getRenderer().getVertexGeometryBuffer(), 0);
		cmd.bindVertexBuffer(1, m_instanceBuffer[frameIndex.value()], 0);
		cmd.bindIndexBuffer(getRenderer().getIndexGeometryBuffer(), gfx::IndexFormat::UnsignedInt, 0);
		// TODO upload command on GPU & use indirect.
		for (gfx::DrawIndexedIndirectCommand& batch : m_drawIndexedBuffer)
		{
			cmd.drawIndexed(batch.indexCount, batch.firstIndex, batch.vertexOffset, batch.instanceCount, batch.firstInstance);
		}
	}
}

InstanceHandle StaticMeshInstanceRenderer::createInstance(AssetID assetID)
{
	using InstanceHandleType = std::underlying_type<InstanceHandle>::type;
	InstanceHandle instanceHandle = static_cast<InstanceHandle>(geometry::random<InstanceHandleType>(0, std::numeric_limits<InstanceHandleType>::max()));
	AKA_ASSERT(m_instanceIndex.find(instanceHandle) == m_instanceIndex.end(), "Hash collision");

	ResourceHandle<StaticMesh> meshHandle = getRenderer().getLibrary()->get<StaticMesh>(assetID);
	const StaticMesh& mesh = meshHandle.get();
	bool dirtyAsset = false;
	auto itAsset = m_assetIndex.find(assetID);
	if (itAsset == m_assetIndex.end())
	{
		dirtyAsset = true;
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
			AKA_ASSERT(m_instanceBatchDatas.size() < MaxBatchCount, "Too many batches");
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
	if (dirtyAsset)
	{
		for (uint32_t i = 0; i < gfx::MaxFrameInFlight; i++)
			m_dirtyAsset[i] = true;
	}
	for (uint32_t i =0; i< gfx::MaxFrameInFlight; i++)
		m_dirtyInstance[i] = true;

	return instanceHandle;
}
void StaticMeshInstanceRenderer::updateInstanceTransform(InstanceHandle instanceHandle, const mat4f& transform)
{
	auto it = m_instanceIndex.find(instanceHandle);
	AKA_ASSERT(it != m_instanceIndex.end(), "");

	m_instanceDatas[it->second].setTransform(transform);

	for (uint32_t i = 0; i < gfx::MaxFrameInFlight; i++)
		m_dirtyInstance[i] = true;
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
		m_dirtyInstance[i] = true;
}

}