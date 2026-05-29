// RenderQueue.cpp

#include "RenderQueue.hpp"
#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include "Renderer.hpp"
#include "ShaderManager.hpp"

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace eHazGraphics {

struct BatchKey {
  ShaderComboID shader;

  uint32_t firstIndex;
  uint32_t baseVertex;
  uint32_t indexCount;

  bool operator==(const BatchKey &other) const {
    return shader == other.shader && firstIndex == other.firstIndex &&
           baseVertex == other.baseVertex && indexCount == other.indexCount;
  }
};

struct BatchKeyHash {
  size_t operator()(const BatchKey &k) const {
    size_t h = 0;

    auto HashCombine = [&h](size_t v) {
      h ^= v + 0x9e3779b9 + (h << 6) + (h >> 2);
    };

    HashCombine(std::hash<uint64_t>()(k.shader.vertex));
    HashCombine(std::hash<uint64_t>()(k.shader.fragment));

    HashCombine(std::hash<uint32_t>()(k.firstIndex));
    HashCombine(std::hash<uint32_t>()(k.baseVertex));
    HashCombine(std::hash<uint32_t>()(k.indexCount));

    return h;
  }
};

bool RenderQueue::Initialize(BufferManager *f_bufferManager) {
  bufferManager = f_bufferManager;
  return true;
}

int RenderQueue::CreateRenderCommand(const VertexIndexInfoPair &ranges,
                                     bool isStatic, unsigned int instanceDataID,
                                     unsigned int instanceCount,
                                     ShaderComboID shaderID) {
  const SBufferRange &vertexRange = ranges.first;
  const SBufferRange &indexRange = ranges.second;

  auto vAlloc = bufferManager->GetAllocation(vertexRange);
  auto iAlloc = bufferManager->GetAllocation(indexRange);

  DrawElementsIndirectCommand command{};

  command.count = indexRange.count;
  command.instanceCount = instanceCount;
  command.firstIndex = iAlloc->offset / sizeof(GLuint);
  command.baseVertex = vAlloc->offset / sizeof(Vertex);
  command.baseInstance = instanceDataID;

  std::pair<DrawElementsIndirectCommand, ShaderComboID> cmd = {command,
                                                               shaderID};

  if (isStatic) {
    StaticCommands.push_back(cmd);
    return static_cast<int>(StaticCommands.size() - 1);
  }

  DynamicCommands.push_back(cmd);
  return static_cast<int>(DynamicCommands.size() - 1);
}

static void SortCommandsByShader(
    std::vector<std::pair<DrawElementsIndirectCommand, ShaderComboID>>
        &sortedCommandPairs) {
  std::stable_sort(sortedCommandPairs.begin(), sortedCommandPairs.end(),
                   [](const auto &a, const auto &b) {
                     if (a.second.vertex != b.second.vertex)
                       return a.second.vertex < b.second.vertex;

                     return a.second.fragment < b.second.fragment;
                   });
}

std::vector<DrawRange> RenderQueue::SubmitRenderCommands() {
  std::vector<std::pair<DrawElementsIndirectCommand, ShaderComboID>>
      sortedCommandPairs;

  sortedCommandPairs.reserve(StaticCommands.size() + DynamicCommands.size());

  for (auto &cmd : StaticCommands)
    sortedCommandPairs.push_back(cmd);

  for (auto &cmd : DynamicCommands)
    sortedCommandPairs.push_back(cmd);

  if (sortedCommandPairs.empty())
    return {};

  // =========================================================
  // SORT BY SHADER
  // =========================================================

  SortCommandsByShader(sortedCommandPairs);

  // =========================================================
  // BATCH COMPATIBLE DRAWS
  // =========================================================

  std::unordered_map<BatchKey, DrawElementsIndirectCommand, BatchKeyHash>
      batches;

  for (auto &pair : sortedCommandPairs) {
    const auto &cmd = pair.first;
    const auto &shader = pair.second;

    BatchKey key{shader, cmd.firstIndex, static_cast<uint32_t>(cmd.baseVertex),
                 cmd.count};

    auto it = batches.find(key);

    if (it == batches.end()) {
      batches.emplace(key, cmd);
    } else {
      // Merge instances
      it->second.instanceCount += cmd.instanceCount;
    }
  }

  // =========================================================
  // BUILD FINAL COMMAND BUFFER
  // =========================================================

  std::vector<DrawElementsIndirectCommand> allCommands;
  std::vector<ShaderComboID> drawCallShaders;

  allCommands.reserve(batches.size());
  drawCallShaders.reserve(batches.size());

  for (auto &batch : batches) {
    allCommands.push_back(batch.second);
    drawCallShaders.push_back(batch.first.shader);
  }

  // =========================================================
  // SORT AGAIN SO SHADER RANGES ARE CONTIGUOUS
  // =========================================================

  std::vector<std::pair<DrawElementsIndirectCommand, ShaderComboID>> finalPairs;

  finalPairs.reserve(allCommands.size());

  for (size_t i = 0; i < allCommands.size(); ++i) {
    finalPairs.push_back({allCommands[i], drawCallShaders[i]});
  }

  SortCommandsByShader(finalPairs);

  allCommands.clear();
  drawCallShaders.clear();

  for (auto &p : finalPairs) {
    allCommands.push_back(p.first);
    drawCallShaders.push_back(p.second);
  }

  // =========================================================
  // BUILD DRAW RANGES
  // =========================================================

  std::vector<DrawRange> drawRanges;

  {
    size_t start = 0;
    ShaderComboID currentShader = finalPairs[0].second;

    for (size_t i = 1; i < finalPairs.size(); ++i) {
      if (finalPairs[i].second != currentShader) {
        drawRanges.push_back({start, i - start, currentShader});

        start = i;
        currentShader = finalPairs[i].second;
      }
    }

    drawRanges.push_back({start, finalPairs.size() - start, currentShader});
  }

  // =========================================================
  // UPLOAD INDIRECT COMMAND BUFFER
  // =========================================================

  numCommands = static_cast<unsigned int>(allCommands.size());

  size_t requiredSize =
      allCommands.size() * sizeof(DrawElementsIndirectCommand);

  // Better:
  // Replace this with persistent mapped buffer later

  bufferLocation = Renderer::p_bufferManager->InsertNewDynamicData(
      allCommands.data(), requiredSize, TypeFlags::BUFFER_DRAW_CALL_DATA);

  return drawRanges;
}

void RenderQueue::ClearDynamicCommands() {
  DynamicCommands.clear();
  numCommands = 0;
}

void RenderQueue::ClearStaticCommnads() {
  StaticCommands.clear();
  numCommands = 0;
}

void RenderQueue::Destroy() {}

bool RenderQueue::UpdateDynamicCommand(
    const std::pair<DrawElementsIndirectCommand, ShaderComboID> &ID,

    std::pair<DrawElementsIndirectCommand, ShaderComboID> replacement) {
  for (unsigned int i = 0; i < DynamicCommands.size(); i++) {
    if (DynamicCommands[i] == ID) {
      DynamicCommands[i] = replacement;
      return true;
    }
  }

  return false;
}

} // namespace eHazGraphics
