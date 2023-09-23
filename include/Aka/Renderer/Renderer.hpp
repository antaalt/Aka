#pragma once

#include <Aka/Core/Container/Vector.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Resource/AssetLibrary.hpp>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Renderer/Instance.hpp>
#include <Aka/Renderer/View.hpp>

namespace aka {

enum class SamplerType
{
	Nearest,
	Bilinear,
	Trilinear,
	Anisotropic,

	First = Nearest,
	Last = Anisotropic,
};

struct BufferOffset
{
	gfx::BufferHandle handle;
	uint32_t offset;
	uint32_t size;
};

class Renderer
{
public:
	Renderer(gfx::GraphicDevice* _device, AssetLibrary* _library);
	~Renderer();

	void create();
	void destroy();

public: // Allocate 
	BufferOffset allocate(void* data, size_t size);
	void update(const BufferOffset& handle, void* data, size_t size, size_t offset = 0);
	void deallocate(const BufferOffset& handle);

public: // Instances
	// Register an instance for a specific asset.
	Instance* createInstance(AssetID assetID);
	void destroyInstance(Instance* instance);

	void render(gfx::Frame* frame);

public:
	View* createView(ViewType type);
	void destroyView(View* view);

public:
	AssetLibrary* getLibrary();
	gfx::GraphicDevice* getDevice() { return m_device; }
	gfx::SamplerHandle getSampler(SamplerType type);
private:
	AssetLibrary* m_library;
	gfx::GraphicDevice* m_device;
	std::map<AssetID, Vector<Instance*>> m_assetInstances[EnumCount<InstanceType>()];

	struct RenderData
	{
		gfx::BufferHandle m_instanceBuffer; // one data struct per type
		gfx::BufferHandle m_instanceBufferStaging;
		gfx::GraphicPipelineHandle m_pipeline;
		//gfx::RenderPassHandle m_renderPass;
	};
	RenderData m_renderData[EnumCount<InstanceType>()];
	// Backbuffer
	gfx::BackbufferHandle m_backbuffer;
	gfx::RenderPassHandle m_backbufferRenderPass;
private:
	gfx::BufferHandle m_viewBuffers;
	Vector<gfx::DescriptorSetHandle> m_viewDescriptorSet;
private:
	Vector<View*> m_views;
private:
	gfx::SamplerHandle m_defaultSamplers[EnumCount<SamplerType>()];
	gfx::BufferHandle m_geometryBuffer;
};



};