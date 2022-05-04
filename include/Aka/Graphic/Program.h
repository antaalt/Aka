#pragma once

#include <stdint.h>

#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

enum class ShaderBindingType : uint8_t
{
	None,
	SampledImage,
	StorageImage,
	UniformBuffer,
	StorageBuffer,
	AccelerationStructure,
};

struct ShaderBindingLayout
{
	ShaderBindingType type; // Type of binding
	ShaderType shaderType; // Shader stage used
	uint32_t count; // Number of element for this binding
};

struct ShaderBindingState
{
	static constexpr uint32_t MaxBindingCount = 16; // TODO increase
	static constexpr uint32_t MaxSetCount = 8;
	ShaderBindingLayout bindings[MaxBindingCount];
	uint32_t count;

	static ShaderBindingState merge(const ShaderBindingState& lhs, const ShaderBindingState& rhs);
};

struct Program : Resource
{
	const Shader* vertex;
	const Shader* fragment;
	const Shader* geometry;
	const Shader* compute;

	ShaderBindingState bindings[ShaderBindingState::MaxSetCount]; // TODO rename sets

	uint32_t setCount;

	bool hasVertexStage() const;
	bool hasFragmentStage() const;
	bool hasGeometryStage() const;
	bool hasComputeStage() const;

	static const Program* createVertex(const Shader* vertex, const Shader* fragment, const ShaderBindingState* bindings, uint32_t bindingCounts);
	static const Program* createGeometry(const Shader* vertex, const Shader* fragment, const Shader* geometry, const ShaderBindingState* bindings, uint32_t bindingCounts);
	static const Program* createCompute(const Shader* compute);
	static void destroy(const Program* program);
};

};
};