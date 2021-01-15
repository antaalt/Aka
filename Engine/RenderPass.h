#pragma once

#include <memory>

namespace aka {

class RenderPass
{
public:
	using Ptr = std::shared_ptr<RenderPass>;

public:
	RenderPass();
	RenderPass(const RenderPass&) = delete;
	RenderPass& operator=(const RenderPass&) = delete;
	~RenderPass();

	static RenderPass::Ptr create();
};

}