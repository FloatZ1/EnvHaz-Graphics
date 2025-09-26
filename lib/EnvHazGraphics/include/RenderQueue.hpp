#ifndef ENVHAZGRAPHICS_RENDER_QUEUE_HPP
#define ENVHAZGRAPHICS_RENDER_QUEUE_HPP


#include "DataStructs.hpp"
#include <vector>
namespace eHazGraphics
{


class RenderQueue
{
  public:
    RenderQueue() = default;


    bool Initialize();

    void CreateRenderCommand(VertexIndexInfoPair offsetData, bool Static);

    void SubmitRenderCommands();


    void ClearDynamicCommands();

    void ClearStaticCommnads();


    void Destroy();


  private:
    std::vector<DrawElementsIndirectCommand> DynamicCommands;
    std::vector<DrawElementsIndirectCommand> StaticCommands;
};


} // namespace eHazGraphics



#endif
