#include <Aka/Graphic/RenderPass.h>

#include <Aka/Core/Application.h>

#include <type_traits>

namespace aka {
namespace gfx {


RenderPass::RenderPass(const char* name, const RenderPassState& state) :
	Resource(name, ResourceType::RenderPass),
	state(state)
{
}

bool operator<(const RenderPassState& lhs, const RenderPassState& rhs)
{
	if (lhs.count < rhs.count) return true;
	else if (lhs.count > rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.colors[i].format < rhs.colors[i].format) return true;
		else if (lhs.colors[i].format > rhs.colors[i].format) return false;
		if (lhs.colors[i].loadOp < rhs.colors[i].loadOp) return true;
		else if (lhs.colors[i].loadOp > rhs.colors[i].loadOp) return false;
		if (lhs.colors[i].storeOp < rhs.colors[i].storeOp) return true;
		else if (lhs.colors[i].storeOp > rhs.colors[i].storeOp) return false;
		if (lhs.colors[i].initialLayout < rhs.colors[i].initialLayout) return true;
		else if (lhs.colors[i].initialLayout > rhs.colors[i].initialLayout) return false;
		if (lhs.colors[i].finalLayout < rhs.colors[i].finalLayout) return true;
		else if (lhs.colors[i].finalLayout > rhs.colors[i].finalLayout) return false;
	}
	if (lhs.depth.format < rhs.depth.format) return true;
	else if (lhs.depth.format > rhs.depth.format) return false;
	if (lhs.depth.loadOp < rhs.depth.loadOp) return true;
	else if (lhs.depth.loadOp > rhs.depth.loadOp) return false;
	if (lhs.depth.storeOp < rhs.depth.storeOp) return true;
	else if (lhs.depth.storeOp > rhs.depth.storeOp) return false;
	if (lhs.depth.initialLayout < rhs.depth.initialLayout) return true;
	else if (lhs.depth.initialLayout > rhs.depth.initialLayout) return false;
	if (lhs.depth.finalLayout < rhs.depth.finalLayout) return true;
	else if (lhs.depth.finalLayout > rhs.depth.finalLayout) return false;
	return false; // equal
}

bool operator>(const RenderPassState& lhs, const RenderPassState& rhs)
{
	if (lhs.count > rhs.count) return true;
	else if (lhs.count < rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.colors[i].format > rhs.colors[i].format) return true;
		else if (lhs.colors[i].format < rhs.colors[i].format) return false;
		if (lhs.colors[i].loadOp > rhs.colors[i].loadOp) return true;
		else if (lhs.colors[i].loadOp < rhs.colors[i].loadOp) return false;
		if (lhs.colors[i].storeOp > rhs.colors[i].storeOp) return true;
		else if (lhs.colors[i].storeOp < rhs.colors[i].storeOp) return false;
		if (lhs.colors[i].initialLayout > rhs.colors[i].initialLayout) return true;
		else if (lhs.colors[i].initialLayout < rhs.colors[i].initialLayout) return false;
		if (lhs.colors[i].finalLayout > rhs.colors[i].finalLayout) return true;
		else if (lhs.colors[i].finalLayout < rhs.colors[i].finalLayout) return false;
	}
	if (lhs.depth.format > rhs.depth.format) return true;
	else if (lhs.depth.format < rhs.depth.format) return false;
	if (lhs.depth.loadOp > rhs.depth.loadOp) return true;
	else if (lhs.depth.loadOp < rhs.depth.loadOp) return false;
	if (lhs.depth.storeOp > rhs.depth.storeOp) return true;
	else if (lhs.depth.storeOp < rhs.depth.storeOp) return false;
	if (lhs.depth.initialLayout > rhs.depth.initialLayout) return true;
	else if (lhs.depth.initialLayout < rhs.depth.initialLayout) return false;
	if (lhs.depth.finalLayout > rhs.depth.finalLayout) return true;
	else if (lhs.depth.finalLayout < rhs.depth.finalLayout) return false;
	return false; // equal
}

bool operator==(const RenderPassState& lhs, const RenderPassState& rhs)
{
	if (lhs.count != rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.colors[i].format != rhs.colors[i].format) return false;
		if (lhs.colors[i].loadOp != rhs.colors[i].loadOp) return false;
		if (lhs.colors[i].storeOp != rhs.colors[i].storeOp) return false;
		if (lhs.colors[i].initialLayout != rhs.colors[i].initialLayout) return false;
		if (lhs.colors[i].finalLayout != rhs.colors[i].finalLayout) return false;
	}
	if (lhs.depth.format != rhs.depth.format) return false;
	if (lhs.depth.loadOp != rhs.depth.loadOp) return false;
	if (lhs.depth.storeOp != rhs.depth.storeOp) return false;
	if (lhs.depth.initialLayout != rhs.depth.initialLayout) return false;
	if (lhs.depth.finalLayout != rhs.depth.finalLayout) return false;
	return true; // equal
}

bool operator!=(const RenderPassState& lhs, const RenderPassState& rhs)
{
	if (lhs.count != rhs.count) return true;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.colors[i].format != rhs.colors[i].format) return true;
		if (lhs.colors[i].loadOp != rhs.colors[i].loadOp) return true;
		if (lhs.colors[i].storeOp != rhs.colors[i].storeOp) return true;
		if (lhs.colors[i].initialLayout != rhs.colors[i].initialLayout) return true;
		if (lhs.colors[i].finalLayout != rhs.colors[i].finalLayout) return true;
	}
	if (lhs.depth.format != rhs.depth.format) return true;
	if (lhs.depth.loadOp != rhs.depth.loadOp) return true;
	if (lhs.depth.storeOp != rhs.depth.storeOp) return true;
	if (lhs.depth.initialLayout != rhs.depth.initialLayout) return true;
	if (lhs.depth.finalLayout != rhs.depth.finalLayout) return true;
	return false; // equal
}

};
};