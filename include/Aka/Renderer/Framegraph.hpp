#pragma once

namespace aka {


namespace fg {

class RenderTask
{
	RenderTask* m_parent;
};

class GraphicTask : RenderTask
{

};

class ComputeTask : RenderTask
{

};
class CopyTask : RenderTask
{

};

} // fg

};