#include "RenderQueue.hpp"
#include "DataStructs.hpp"
#include "Renderer.hpp"
#include "ShaderManager.hpp"

#include <algorithm>
#include <vector>


namespace eHazGraphics
{




bool RenderQueue::Initialize()
{




    return true;
}
int RenderQueue::CreateRenderCommand(VertexIndexInfoPair offsetData, bool Static, unsigned int InstanceDataID,
                                     unsigned int InstanceCount, ShaderComboID shaderID)
{
    DrawElementsIndirectCommand command;

    command.baseVertex = offsetData.first.offset;
    command.firstIndex = offsetData.second.offset;
    command.count = offsetData.second.count;
    command.baseInstance = InstanceDataID;
    command.instanceCount = InstanceCount;

    std::pair<DrawElementsIndirectCommand, ShaderComboID> cmd = {command, shaderID};

    if (Static)
    {
        StaticCommands.push_back(cmd);
        return StaticCommands.size() - 1;
    }

    else
    {
        DynamicCommands.push_back(cmd);
        return DynamicCommands.size() - 1;
    }
}



void SortCommandsByShader(std::vector<std::pair<DrawElementsIndirectCommand, ShaderComboID>> &sortedCommandPairs)
{
    std::stable_sort(sortedCommandPairs.begin(), sortedCommandPairs.end(),
                     [](const std::pair<DrawElementsIndirectCommand, ShaderComboID> &a,
                        const std::pair<DrawElementsIndirectCommand, ShaderComboID> &b) {
                         // First compare vertex shader hash
                         if (a.second.vertex != b.second.vertex)
                             return a.second.vertex < b.second.vertex;

                         // Then compare fragment shader hash
                         return a.second.fragment < b.second.fragment;
                     });
}

std::vector<DrawRange> CreateDrawRanges(
    std::vector<std::pair<DrawElementsIndirectCommand, ShaderComboID>> &sortedCommandPairs)
{

    std::vector<DrawRange> result;


    size_t start = 0;
    ShaderComboID current = sortedCommandPairs[0].second;


    for (size_t i = 0; i < sortedCommandPairs.size(); ++i)
    {

        if (sortedCommandPairs[i].second != current)
        {
            result.push_back({start, i - start, current});
            start = i;
            current = sortedCommandPairs[i].second;
        }
    }


    result.push_back({start, sortedCommandPairs.size() - start, current});




    return result;
}

std::vector<DrawRange> RenderQueue::SubmitRenderCommands()
{
    std::vector<ShaderComboID> DrawCallShaders;
    std::vector<DrawElementsIndirectCommand> allCommands;

    std::vector<std::pair<DrawElementsIndirectCommand, ShaderComboID>> sortedCommandPairs;

    for (auto cmd : StaticCommands)
    {
        sortedCommandPairs.push_back(cmd);
    }
    for (auto cmd : DynamicCommands)
    {
        sortedCommandPairs.push_back(cmd);
    }


    SortCommandsByShader(sortedCommandPairs);

    numCommands = sortedCommandPairs.size();

    auto fullSize = DynamicCommands.size() + StaticCommands.size();
    allCommands.reserve(fullSize);
    DrawCallShaders.reserve(fullSize);


    for (auto &pair : sortedCommandPairs)
    {
        allCommands.push_back(pair.first);
        DrawCallShaders.push_back(pair.second);
    }






    std::vector<DrawRange> drawRange;


    drawRange = CreateDrawRanges(sortedCommandPairs);



    size_t requiredSize = allCommands.size() * sizeof(DrawElementsIndirectCommand);

    /* if (numCommands == previousNumCommands && bufferLocation.size >= requiredSize)
     {
         Renderer::p_bufferManager->UpdateData(bufferLocation, allCommands.data(), requiredSize);
     }
     else
     {
         Renderer::p_bufferManager->ClearBuffer(TypeFlags::BUFFER_DRAW_CALL_DATA);
         bufferLocation = Renderer::p_bufferManager->InsertNewDynamicData(allCommands.data(), requiredSize,
                                                                          TypeFlags::BUFFER_DRAW_CALL_DATA);
         previousNumCommands = numCommands;
     }*/
    if (numCommands == previousNumCommands && bufferLocation.size > 0)
    {
        Renderer::p_bufferManager->UpdateData(bufferLocation, allCommands.data(),
                                              allCommands.size() * sizeof(DrawElementsIndirectCommand));
    }
    else
    {
        bufferLocation = Renderer::p_bufferManager->InsertNewDynamicData(
            allCommands.data(), allCommands.size() * sizeof(DrawElementsIndirectCommand),
            TypeFlags::BUFFER_DRAW_CALL_DATA);
        previousNumCommands = numCommands;
    }

    return drawRange;
}



void RenderQueue::ClearDynamicCommands()
{
    DynamicCommands.clear();
}
void RenderQueue::ClearStaticCommnads()
{
    StaticCommands.clear();
}
void RenderQueue::Destroy()
{
}

bool RenderQueue::UpdateDynamicCommand(const std::pair<DrawElementsIndirectCommand, ShaderComboID> &ID,
                                       std::pair<DrawElementsIndirectCommand, ShaderComboID> replacement)
{

    for (unsigned int i = 0; i < DynamicCommands.size(); i++)
    {

        if (DynamicCommands[i] == ID)
        {
            DynamicCommands[i] = replacement;
            return true;
        }
    }
    return false;
}
} // namespace eHazGraphics
