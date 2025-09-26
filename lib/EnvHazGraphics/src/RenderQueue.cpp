#include "RenderQueue.hpp"
#include "DataStructs.hpp"


namespace eHazGraphics
{




bool RenderQueue::Initialize()
{




    return false;
}
void RenderQueue::CreateRenderCommand(VertexIndexInfoPair offsetData, bool Static)
{
    DrawElementsIndirectCommand command;
}
void RenderQueue::SubmitRenderCommands()
{
}
void RenderQueue::ClearDynamicCommands()
{
}
void RenderQueue::ClearStaticCommnads()
{
}
void RenderQueue::Destroy()
{
}

} // namespace eHazGraphics
