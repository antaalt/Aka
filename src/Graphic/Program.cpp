#include <Aka/Graphic/Program.h>

#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

bool ShaderPipelineLayout::isValid() const
{
	if (setCount > ShaderMaxSetCount)
		return false;
	for (uint32_t i = 0; i < setCount; i++)
	{
		if (sets[i].count > ShaderMaxBindingCount)
			return false;
	}
	if (constantCount > ShaderMaxConstantCount)
		return false;
	return true;
}

Program::Program(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderPipelineLayout& layout) :
	Resource(name, ResourceType::Program),
	shaders{},
	layout(layout)
{
	shaders[EnumToIndex(ShaderType::Vertex)] = vertex;
	shaders[EnumToIndex(ShaderType::Fragment)] = fragment;
	AKA_ASSERT(layout.isValid(), "Shader pipeline layout not valid");
}

Program::Program(const char* name, ShaderHandle task, ShaderHandle mesh, ShaderHandle fragment, const ShaderPipelineLayout& layout) :
	Resource(name, ResourceType::Program),
	shaders{},
	layout(layout)
{
	shaders[EnumToIndex(ShaderType::Task)] = task;
	shaders[EnumToIndex(ShaderType::Mesh)] = mesh;
	shaders[EnumToIndex(ShaderType::Fragment)] = fragment;
	AKA_ASSERT(layout.isValid(), "Shader pipeline layout not valid");
}
Program::Program(const char* name, ShaderHandle compute, const ShaderPipelineLayout& layout) :
	Resource(name, ResourceType::Program),
	shaders{},
	layout(layout)
{
	shaders[EnumToIndex(ShaderType::Compute)] = compute;
	AKA_ASSERT(layout.isValid(), "Shader pipeline layout not valid");
}

bool operator<(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	if (lhs.count < rhs.count) return true;
	else if (lhs.count > rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.bindings[i].flags < rhs.bindings[i].flags) return true;
		else if (lhs.bindings[i].flags > rhs.bindings[i].flags) return false;
		if (lhs.bindings[i].count < rhs.bindings[i].count) return true;
		else if (lhs.bindings[i].count > rhs.bindings[i].count) return false;
		if (lhs.bindings[i].stages < rhs.bindings[i].stages) return true;
		else if (lhs.bindings[i].stages > rhs.bindings[i].stages) return false;
		if (lhs.bindings[i].type < rhs.bindings[i].type) return true;
		else if (lhs.bindings[i].type > rhs.bindings[i].type) return false;
	}
	return false; // equal
}
bool operator>(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	if (lhs.count > rhs.count) return true;
	else if (lhs.count < rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.bindings[i].flags > rhs.bindings[i].flags) return true;
		else if (lhs.bindings[i].flags < rhs.bindings[i].flags) return false;
		if (lhs.bindings[i].count > rhs.bindings[i].count) return true;
		else if (lhs.bindings[i].count < rhs.bindings[i].count) return false;
		if (lhs.bindings[i].stages > rhs.bindings[i].stages) return true;
		else if (lhs.bindings[i].stages < rhs.bindings[i].stages) return false;
		if (lhs.bindings[i].type > rhs.bindings[i].type) return true;
		else if (lhs.bindings[i].type < rhs.bindings[i].type) return false;
	}
	return false; // equal
}
bool operator==(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	if (lhs.count != rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.bindings[i].flags != rhs.bindings[i].flags) return false;
		if (lhs.bindings[i].count != rhs.bindings[i].count) return false;
		if (lhs.bindings[i].stages != rhs.bindings[i].stages) return false;
		if (lhs.bindings[i].type != rhs.bindings[i].type) return false;
	}
	return true; // equal
}
bool operator!=(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	return !operator==(lhs, rhs);
}

bool operator<(const ShaderConstant& lhs, const ShaderConstant& rhs)
{
	if (lhs.offset < rhs.offset) return true;
	else if (lhs.offset > rhs.offset) return false;
	if (lhs.size < rhs.size) return true;
	else if (lhs.size > rhs.size) return false;
	if (lhs.shader < rhs.shader) return true;
	else if (lhs.shader > rhs.shader) return false;
	return false; // equal
}
bool operator>(const ShaderConstant& lhs, const ShaderConstant& rhs)
{
	if (lhs.offset > rhs.offset) return true;
	else if (lhs.offset < rhs.offset) return false;
	if (lhs.size > rhs.size) return true;
	else if (lhs.size < rhs.size) return false;
	if (lhs.shader > rhs.shader) return true;
	else if (lhs.shader < rhs.shader) return false;
	return false; // equal
}
bool operator==(const ShaderConstant& lhs, const ShaderConstant& rhs)
{
	return lhs.offset == rhs.offset && lhs.size == rhs.size && lhs.shader == rhs.shader;
}
bool operator!=(const ShaderConstant& lhs, const ShaderConstant& rhs)
{
	return !operator==(lhs, rhs);
}

bool operator<(const ShaderPipelineLayout& lhs, const ShaderPipelineLayout& rhs)
{
	if (lhs.constantCount < rhs.constantCount) return true;
	else if (lhs.constantCount > rhs.constantCount) return true;
	if (lhs.setCount < rhs.setCount) return true;
	else if (lhs.setCount > rhs.setCount) return true;
	for (uint32_t i = 0; i < rhs.setCount; i++)
	{
		if (lhs.sets[i] < rhs.sets[i]) return true;
		else if (lhs.sets[i] > rhs.sets[i]) return true;
	}
	for (uint32_t i = 0; i < rhs.constantCount; i++)
	{
		if (lhs.constants[i] < rhs.constants[i]) return true;
		else if (lhs.constants[i] > rhs.constants[i]) return true;
	}
	return false; // equal
}
bool operator>(const ShaderPipelineLayout& lhs, const ShaderPipelineLayout& rhs)
{
	if (lhs.constantCount > rhs.constantCount) return true;
	else if (lhs.constantCount < rhs.constantCount) return true;
	if (lhs.setCount > rhs.setCount) return true;
	else if (lhs.setCount < rhs.setCount) return true;
	for (uint32_t i = 0; i < rhs.setCount; i++)
	{
		if (lhs.sets[i] > rhs.sets[i]) return true;
		else if (lhs.sets[i] < rhs.sets[i]) return true;
	}
	for (uint32_t i = 0; i < rhs.constantCount; i++)
	{
		if (lhs.constants[i] > rhs.constants[i]) return true;
		else if (lhs.constants[i] < rhs.constants[i]) return true;
	}
	return false; // equal
}
bool operator==(const ShaderPipelineLayout& lhs, const ShaderPipelineLayout& rhs)
{
	bool equal = true;
	equal |= lhs.constantCount == rhs.constantCount;
	equal |= lhs.setCount == rhs.setCount;
	for (uint32_t i = 0; i < rhs.setCount; i++)
		equal |= lhs.sets[i] == rhs.sets[i];
	for (uint32_t i = 0; i < rhs.constantCount; i++)
		equal |= lhs.constants[i] == rhs.constants[i];
	return equal;
}
bool operator!=(const ShaderPipelineLayout& lhs, const ShaderPipelineLayout& rhs)
{
	return !operator==(lhs, rhs);
}

bool Program::hasShaderStage(ShaderType _shaderType) const
{
	return shaders[EnumToIndex(_shaderType)] != ShaderHandle::null;
}
bool Program::hasVertexStage() const
{
	return hasShaderStage(ShaderType::Vertex);
}
bool Program::hasFragmentStage() const
{
	return hasShaderStage(ShaderType::Fragment);
}
bool Program::hasMeshStage() const
{
	return hasShaderStage(ShaderType::Mesh);
}
bool Program::hasTaskStage() const
{
	return hasShaderStage(ShaderType::Task);
}
bool Program::hasComputeStage() const
{
	return hasShaderStage(ShaderType::Compute);
}

bool Program::isCompatible(const ShaderPipelineLayout& _layout) const
{
	bool compatible = true;
	compatible &= layout.constantCount == _layout.constantCount;
	compatible &= layout.setCount == _layout.setCount;
	for (uint32_t iSet = 0; iSet < layout.setCount; iSet++)
	{
		for (uint32_t iBinding = 0; iBinding < layout.sets[iSet].count; iBinding++)
		{
			const ShaderBindingLayout& layoutBinding = _layout.sets[iSet].bindings[iBinding];
			const ShaderBindingLayout& shaderBinding = layout.sets[iSet].bindings[iBinding];
			compatible &= shaderBinding.type == layoutBinding.type;
			// Cant retrieve bindless count from spirv-cross, so we ignore it here for flag & count
			compatible &= shaderBinding.flags == layoutBinding.flags || asBool(layoutBinding.flags & ShaderBindingFlag::Bindless);
			compatible &= shaderBinding.count == layoutBinding.count || asBool(layoutBinding.flags & ShaderBindingFlag::Bindless);
			compatible &= (shaderBinding.stages & layoutBinding.stages) == shaderBinding.stages;
		}
	}
	for (uint32_t i = 0; i < layout.constantCount; i++)
	{
		compatible &= layout.constants[i].offset == _layout.constants[i].offset;
		compatible &= layout.constants[i].size == _layout.constants[i].size;
		compatible &= (layout.constants[i].shader & _layout.constants[i].shader) == layout.constants[i].shader;
	}
	return compatible;
}

ProgramHandle Program::createVertex(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderPipelineLayout& layout)
{
	return Application::app()->graphic()->createVertexProgram(name, vertex, fragment, layout);
}
ProgramHandle Program::createMesh(const char* name, ShaderHandle task, ShaderHandle mesh, ShaderHandle fragment, const ShaderPipelineLayout& layout)
{
	return Application::app()->graphic()->createMeshProgram(name, task, mesh, fragment, layout);
}
ProgramHandle Program::createCompute(const char* name, ShaderHandle compute, const ShaderPipelineLayout& layout)
{
	return Application::app()->graphic()->createComputeProgram(name, compute, layout);
}
void Program::destroy(ProgramHandle program)
{
	Application::app()->graphic()->destroy(program);
}

};
};