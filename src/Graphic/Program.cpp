#include <Aka/Graphic/Program.h>

#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

bool operator<(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	if (lhs.count < rhs.count) return true;
	else if (lhs.count > rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.bindings[i].count < rhs.bindings[i].count) return true;
		else if (lhs.bindings[i].count > rhs.bindings[i].count) return false;
		if (lhs.bindings[i].shaderType < rhs.bindings[i].shaderType) return true;
		else if (lhs.bindings[i].shaderType > rhs.bindings[i].shaderType) return false;
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
		if (lhs.bindings[i].count > rhs.bindings[i].count) return true;
		else if (lhs.bindings[i].count < rhs.bindings[i].count) return false;
		if (lhs.bindings[i].shaderType > rhs.bindings[i].shaderType) return true;
		else if (lhs.bindings[i].shaderType < rhs.bindings[i].shaderType) return false;
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
		if (lhs.bindings[i].count != rhs.bindings[i].count) return false;
		if (lhs.bindings[i].shaderType != rhs.bindings[i].shaderType) return false;
		if (lhs.bindings[i].type != rhs.bindings[i].type) return false;
	}
	return true; // equal
}
bool operator!=(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	if (lhs.count != rhs.count) return true;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.bindings[i].count != rhs.bindings[i].count) return true;
		if (lhs.bindings[i].shaderType != rhs.bindings[i].shaderType) return true;
		if (lhs.bindings[i].type != rhs.bindings[i].type) return true;
	}
	return false; // equal
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