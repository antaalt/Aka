#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Resource/Resource/SkeletalMesh.hpp>
#include <Aka/Renderer/InstanceRenderer.hpp>
#include <Aka/Renderer/Instance/SkeletalMeshInstance.hpp>

namespace aka {

class SkeletalMeshInstanceRenderer final : public InstanceRenderer
{
public:
	SkeletalMeshInstanceRenderer(Renderer& renderer);
	~SkeletalMeshInstanceRenderer() {}

	void create() override;
	void destroy() override;
	void prepare(gfx::FrameHandle frame) override;
	void render(const View& view, gfx::FrameHandle frame) override;

	InstanceHandle createInstance(AssetID assetID) override;
	void updateBoneInstanceTransform(InstanceHandle instanceHandle, uint32_t boneIndex, const mat4f& transform);
	void updateInstanceTransform(InstanceHandle instanceHandle, const mat4f& transform) override;
	void destroyInstance(InstanceHandle instanceHandle) override;

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
	gfx::ShaderPipelineLayout m_layout;

	gfx::GraphicPipelineHandle m_pipeline;
	ProgramKey m_programKey;
	bool m_dirty[gfx::MaxFrameInFlight];
	bool m_dirtyBones[gfx::MaxFrameInFlight];

private:
	HashMap<AssetID, uint32_t> m_assetIndex;
	HashMap<InstanceHandle, uint32_t> m_instanceIndex;
	// TODO move to gpu with compute.
	gfx::vector<SkeletalMeshInstance> m_instanceDatas; // Updated every frame for bones.
	gfx::vector<gpu::SkeletalMeshAssetData> m_instanceAssetDatas; // Updated rarely
	gfx::vector<gpu::SkeletalMeshBatchData> m_instanceBatchDatas; // Updated rarely

	gfx::Vector<gfx::DrawIndexedIndirectCommand> m_drawIndexedBuffer;

private:
	gfx::BackbufferHandle m_backbuffer;
	gfx::RenderPassHandle m_backbufferRenderPass;
};


};