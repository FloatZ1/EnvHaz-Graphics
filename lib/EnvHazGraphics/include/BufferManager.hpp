#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP

#include "BitFlags.hpp"
#include <DataStructs.hpp>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <algorithm>

#if defined(_WIN32)
#include <Windows.h>

#elif defined(__linux__)
#include <alloca.h>
#endif
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <glad/glad.h>

#include <optional>
#include <ratio>
#include <regex>
#include <vector>

#include "DataStructs.hpp"
#include "DynamicBuffer.hpp"
#include "StaticStack.hpp"

namespace eHazGraphics {

class BufferManager {

public:
  BufferManager();
  // TODO: Update
  BufferManager(BufferManager &&other) noexcept
      : InstanceData(std::move(other.InstanceData)),
        AnimationMatrices(std::move(other.AnimationMatrices)),
        TextureHandleBuffer(std::move(other.TextureHandleBuffer)),
        ParticleData(std::move(other.ParticleData)),
        DrawCommandBuffer(std::move(other.DrawCommandBuffer)),
        StaticMeshInformation(std::move(other.StaticMeshInformation)),
        TerrainBuffer(std::move(other.TerrainBuffer)),
        DynamicBufferIDs(std::move(other.DynamicBufferIDs)),
        StaticbufferIDs(std::move(other.StaticbufferIDs)) {}

  void Initialize(); // NOTE: MAKE INITIAL SIZE VARY DEPENDING ON LAST SESSIONS
                     // MOST MEMORY USED IN EACH

  void SetStaticBufferUsage(bool p_value = true) { m_bUseStack = p_value; };

  bool IsUsingStaticStack() { return m_bUseStack; }

  void BeginWritting();

  void BindDynamicBuffer(TypeFlags type);

  VertexIndexInfoPair InsertNewStaticData(const Vertex *vertexData,
                                          size_t vertexDataSize,
                                          const GLuint *indexData,
                                          size_t indexDataSize, TypeFlags type);

  SBufferRange InsertNewDynamicData(const void *data, size_t size,
                                    TypeFlags type);

  void ClearBuffer(TypeFlags whichBuffer);

  void BindStaticBuffer(TypeFlags buffer) {

    if (m_bUseStack) {
      switch (buffer) {
      case TypeFlags::BUFFER_STATIC_MESH_DATA:
        StaticMeshInformation.BindBuffer();
        break;
      case TypeFlags::BUFFER_STATIC_TERRAIN_DATA:
        TerrainBuffer.BindBuffer();
        break;
      default:
        SDL_Log("Failed to bind static buffer, unknown TypeFlag given to "
                "BindStaticBuffer()\n");
      }
    } else {
      // TODO: IMPLEMENT
    }
  }

  // Removes a range from the dynamic buffer , i recomend you dont use this

  void InvalidateStaticRange(const VertexIndexInfoPair &p_pair) {

    for (auto &buffer : StaticbufferIDs) {
      if (buffer->GetStaticStackID() == p_pair.first.handle.bufferID) {
        buffer->InvalidateRange(p_pair);
      }
    }
  }

  void UpdateData(SBufferRange &range, const void *data, const size_t size);

  std::optional<SAllocation> GetAllocation(const SBufferRange &range) {

    if (m_bUseStack) {

      for (auto &buffer : StaticbufferIDs) {

        if (buffer->GetStaticStackID() == range.handle.bufferID) {

          return buffer->GetAllocation(range);
        }
      }
    } else {

      // TODO: IMPLEMENT
    }
    for (auto &buffer : DynamicBufferIDs) {

      if (buffer->GetBufferID() == range.handle.bufferID) {

        return buffer->GetAllocation(range.handle.allocationID);
      }
    }

    return std::nullopt;
  }

  void WaitForBuffer(TypeFlags buffer);

  std::vector<GLuint> GetGLBufferID(TypeFlags Buffer);

  void EndWritting();

  void UpdateManager();

  void Destroy();

private:
  bool m_bUseStack = true;

  CDynamicBuffer InstanceData;
  CDynamicBuffer AnimationMatrices;
  CDynamicBuffer TextureHandleBuffer;
  CDynamicBuffer ParticleData;
  CDynamicBuffer DrawCommandBuffer;
  CDynamicBuffer cameraMatrices;
  CDynamicBuffer LightsBuffer;
  CDynamicBuffer StaticMatrices;

  CDynamicBuffer DebugShapesVertices;
  CDynamicBuffer DebugShapeIndices;
  CDynamicBuffer DebugShapeMatrices;

  CGLStaticStack StaticMeshInformation;
  CGLStaticStack TerrainBuffer;
  // StaticBuffer StaticMatrices;
  //  Every time a buffer is added update the following functions:
  //  Initialize(), InsertNew*Data() , ClearBuffer() and BitFlags

  // NOTE: ALSO FUCKING BIND THE SHIT IN THE DRAW CALL GOD DAMN.

  // CHANGE THESE EVERYTIME YOU ADD A BUFFER!!!!!!!!!
  unsigned int numOfDynamicBuffers = 10;
  unsigned int numofStaticBuffers = 2;

  std::vector<CGLStaticStack *> StaticbufferIDs;
  std::vector<CDynamicBuffer *> DynamicBufferIDs;

  // std::unordered_map<MeshID, >
};

} // namespace eHazGraphics

#endif
