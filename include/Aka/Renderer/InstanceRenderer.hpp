#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Renderer/View.hpp>
#include <Aka/Renderer/Instance.hpp>

namespace aka {

class Renderer;
class AssetLibrary;

class InstanceRenderer
{
public:
	InstanceRenderer(Renderer& _renderer);
	virtual ~InstanceRenderer() {}

	virtual void create() = 0;
	virtual void destroy() = 0;
	virtual void prepare(const View& view, gfx::FrameHandle _frame) = 0;
	virtual void render(const View& view, gfx::FrameHandle _frame, gfx::RenderPassCommandList& cmd) = 0;

	virtual InstanceHandle createInstance(AssetID assetID) = 0;
	virtual void updateInstanceTransform(InstanceHandle instanceHandle, const mat4f& transform) = 0;
	virtual void destroyInstance(InstanceHandle instanceHandle) = 0;
protected:
	gfx::GraphicDevice* getDevice();
	Renderer& getRenderer();
private:
	Renderer& m_renderer;
};


};