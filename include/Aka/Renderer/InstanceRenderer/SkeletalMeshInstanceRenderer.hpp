#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Resource/Resource/SkeletalMesh.hpp>
#include <Aka/Renderer/InstanceRenderer.hpp>
#include <Aka/Renderer/Instance/SkeletalMeshInstance.hpp>

namespace aka {

class SkeletalMeshInstanceRenderer : public InstanceRenderer
{
public:
	SkeletalMeshInstanceRenderer(Renderer& renderer);

	void create() override;
	void destroy() override;
	void prepare(gfx::FrameHandle frame) override;
	void render(const View& view, gfx::FrameHandle frame) override;
	void resize(uint32_t width, uint32_t height) override;
	void onReceive(const ShaderReloadedEvent& event) override;

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

	gfx::GraphicPipelineHandle m_pipeline;
	ProgramKey m_programKey;
	bool m_dirty[gfx::MaxFrameInFlight];
	bool m_dirtyBones[gfx::MaxFrameInFlight];

private:
	std::map<AssetID, uint32_t> m_assetIndex;
	std::map<InstanceHandle, uint32_t> m_instanceIndex;
	// TODO move to gpu with compute.
	std::vector<SkeletalMeshInstance> m_instanceDatas; // Updated every frame for bones.
	std::vector<gpu::SkeletalMeshAssetData> m_instanceAssetDatas; // Updated rarely
	std::vector<gpu::SkeletalMeshBatchData> m_instanceBatchDatas; // Updated rarely

	Vector<gfx::DrawIndexedIndirectCommand> m_drawIndexedBuffer;

private:
	gfx::BackbufferHandle m_backbuffer;
	gfx::RenderPassHandle m_backbufferRenderPass;
};


};