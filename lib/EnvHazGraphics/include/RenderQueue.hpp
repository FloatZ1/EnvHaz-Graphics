#ifndef ENVHAZGRAPHICS_RENDER_QUEUE_HPP
#define ENVHAZGRAPHICS_RENDER_QUEUE_HPP


#include "DataStructs.hpp"
#include <utility>
#include <vector>
namespace eHazGraphics
{


class RenderQueue
{
  public:
    RenderQueue() = default;

    // nothing to initialize yet just here incase
    bool Initialize();

    int CreateRenderCommand(VertexIndexInfoPair offsetData, bool Static, unsigned int InstanceDataID,
                            unsigned int InstanceCount, ShaderComboID shaderID);


    // Sends the draw commands to the gpu and returns a sorted vector of shaderIDs, each corresponding to
    std::vector<DrawRange> SubmitRenderCommands();




    void ClearDynamicCommands();

    bool UpdateDynamicCommand(const std::pair<DrawElementsIndirectCommand, ShaderComboID> &ID,
                              std::pair<DrawElementsIndirectCommand, ShaderComboID> replacement);

    void ClearStaticCommnads();


    void Destroy();


  private:
    std::vector<std::pair<DrawElementsIndirectCommand, ShaderComboID>> DynamicCommands;
    BufferRange bufferLocation = BufferRange();
    int numCommands = 0;
    int previousNumCommands = 0;

    std::vector<std::pair<DrawElementsIndirectCommand, ShaderComboID>> StaticCommands;
};


} // namespace eHazGraphics



#endif
