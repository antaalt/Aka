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

struct Program;
using ProgramHandle = ResourceHandle<Program>;

struct Program : Resource
{
	ShaderHandle vertex;
	ShaderHandle fragment;
	ShaderHandle geometry;
	ShaderHandle compute;

	ShaderBindingState bindings[ShaderBindingState::MaxSetCount]; // TODO rename sets

	uint32_t setCount;

	bool hasVertexStage() const;
	bool hasFragmentStage() const;
	bool hasGeometryStage() const;
	bool hasComputeStage() const;

	static ProgramHandle createVertex(ShaderHandle vertex, ShaderHandle fragment, const ShaderBindingState* bindings, uint32_t bindingCounts);
	static ProgramHandle createGeometry(ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, const ShaderBindingState* bindings, uint32_t bindingCounts);
	static ProgramHandle createCompute(ShaderHandle compute);
	static void destroy(ProgramHandle program);
};

};
};