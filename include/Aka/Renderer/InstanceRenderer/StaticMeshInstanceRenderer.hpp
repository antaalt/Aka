#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Resource/Resource/StaticMesh.hpp>
#include <Aka/Renderer/InstanceRenderer.hpp>
#include <Aka/Renderer/Instance/StaticMeshInstance.hpp>

namespace aka {

class StaticMeshInstanceRenderer final : public InstanceRenderer
{
public:
	StaticMeshInstanceRenderer(Renderer& renderer);
	~StaticMeshInstanceRenderer() {}

	void create() override;
	void destroy() override;
	void prepare(const View& view, gfx::FrameHandle frame) override;
	void render(const View& view, gfx::FrameHandle frame, gfx::RenderPassCommandList& cmd) override;

	InstanceHandle createInstance(AssetID assetID) override;
	void updateInstanceTransform(InstanceHandle instanceHandle, const mat4f& transform) override;
	void destroyInstance(InstanceHandle instanceHandle) override;
private:
	void createPipeline();
	void destroyPipeline();
private:
	gfx::BufferHandle m_assetBuffer;
	gfx::BufferHandle m_assetBufferStaging;
	gfx::BufferHandle m_batchBuffer;
	gfx::BufferHandle m_batchBufferStaging;
	gfx::BufferHandle m_instanceBufferStaging;
	gfx::BufferHandle m_instanceBuffer[gfx::MaxFrameInFlight];
	gfx::DescriptorPoolHandle m_modelDescriptorPool;
	gfx::DescriptorSetHandle m_modelDescriptorSet;
	gfx::ShaderPipelineLayout m_layout;

	gfx::GraphicPipelineHandle m_pipeline;
	ProgramKey m_programKey;
	bool m_dirtyInstance[gfx::MaxFrameInFlight];
	bool m_dirtyAsset[gfx::MaxFrameInFlight];

private:
	HashMap<AssetID, uint32_t> m_assetIndex;
	HashMap<InstanceHandle, uint32_t> m_instanceIndex;
	// TODO move to gpu with compute.
	gfx::vector<StaticMeshInstance> m_instanceDatas; // Updated frequently.
	gfx::vector<gpu::StaticMeshAssetData> m_instanceAssetDatas; // Updated rarely
	gfx::vector<gpu::StaticMeshBatchData> m_instanceBatchDatas; // Updated rarely

	gfx::Vector<gfx::DrawIndexedIndirectCommand> m_drawIndexedBuffer;
};


};