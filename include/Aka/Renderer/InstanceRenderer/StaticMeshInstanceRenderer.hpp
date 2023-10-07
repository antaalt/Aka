#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Resource/Resource/StaticMesh.hpp>
#include <Aka/Renderer/InstanceRenderer.hpp>
#include <Aka/Renderer/Instance/StaticMeshInstance.hpp>

namespace aka {

class StaticMeshInstanceRenderer : public InstanceRenderer
{
public:
	StaticMeshInstanceRenderer(Renderer& renderer);

	void create() override;
	void destroy() override;
	void prepare(gfx::FrameHandle frame) override;
	void render(const View& view, gfx::FrameHandle frame) override;
	void resize(uint32_t width, uint32_t height) override;
	void onReceive(const ShaderReloadedEvent& event) override;

	InstanceHandle createInstance(AssetID assetID);
	void updateInstanceTransform(InstanceHandle instanceHandle, const mat4f& transform);
	void destroyInstance(InstanceHandle instanceHandle);
private:
	void createPipeline();
	void destroyPipeline();
private:
	gfx::BufferHandle m_assetBuffer;
	gfx::BufferHandle m_batchBuffer;
	gfx::BufferHandle m_instanceBufferStaging;
	gfx::BufferHandle m_instanceBuffer[gfx::MaxFrameInFlight];
	gfx::DescriptorPoolHandle m_modelDescriptorPool;
	gfx::DescriptorSetHandle m_modelDescriptorSet;

	gfx::GraphicPipelineHandle m_pipeline;
	ProgramKey m_programKey;
	bool m_dirty[gfx::MaxFrameInFlight];

private:
	std::map<AssetID, uint32_t> m_assetIndex;
	std::map<InstanceHandle, uint32_t> m_instanceIndex;
	// TODO move to gpu with compute.
	std::vector<StaticMeshInstance> m_instanceDatas; // Updated frequently.
	std::vector<gpu::StaticMeshAssetData> m_instanceAssetDatas; // Updated rarely
	std::vector<gpu::StaticMeshBatchData> m_instanceBatchDatas; // Updated rarely

	Vector<gfx::DrawIndexedIndirectCommand> m_drawIndexedBuffer;

private:
	gfx::BackbufferHandle m_backbuffer;
	gfx::RenderPassHandle m_backbufferRenderPass;
};


};