#include <Aka/Graphic/Program.h>

#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

ShaderBindingState ShaderBindingState::merge(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	ShaderBindingState bindings = lhs;
	for (uint32_t i = 0; i < rhs.count; i++)
	{
		if (rhs.bindings[i].type != ShaderBindingType::None)
		{
			if (lhs.bindings[i].type == ShaderBindingType::None)
			{
				bindings.bindings[i] = rhs.bindings[i];
				bindings.count = max(bindings.count, i + 1);
			}
			else
			{
				AKA_ASSERT(rhs.bindings[i].type == lhs.bindings[i].type, "Mismatching bindings");
				AKA_ASSERT(rhs.bindings[i].count == lhs.bindings[i].count, "Mismatching count");
				bindings.bindings[i].shaderType = bindings.bindings[i].shaderType | rhs.bindings[i].shaderType;
			}
		}
	}
	return bindings;
}

bool Program::hasVertexStage() const
{
	return vertex.data != nullptr;
}
bool Program::hasFragmentStage() const
{
	return fragment.data != nullptr;
}
bool Program::hasGeometryStage() const
{
	return geometry.data != nullptr;
}
bool Program::hasComputeStage() const
{
	return compute.data != nullptr;
}

ProgramHandle Program::createVertex(ShaderHandle vertex, ShaderHandle fragment, const ShaderBindingState* bindings, uint32_t count)
{
	return Application::app()->graphic()->createProgram(vertex, fragment, ShaderHandle::null, bindings, count);
}
ProgramHandle Program::createGeometry(ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, const ShaderBindingState* bindings, uint32_t count)
{
	return Application::app()->graphic()->createProgram(vertex, fragment, geometry, bindings, count);
}
ProgramHandle Program::createCompute(ShaderHandle compute)
{
	return ProgramHandle::null;
}
void Program::destroy(ProgramHandle program)
{
	Application::app()->graphic()->destroy(program);
}

};
};