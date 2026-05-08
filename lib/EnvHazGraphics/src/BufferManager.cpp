#include "BufferManager.hpp"
#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "DynamicBuffer.hpp"
#include "StaticStack.hpp"
#include "glad/glad.h"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_stdinc.h>
#include <algorithm>
// #include <bits/types/mbstate_t.h>
#include <cassert>

#include <cstddef>

#include <cstdint>
#include <cstring>
#include <iterator>

#include <utility>
#include <vector>

namespace eHazGraphics {

BufferManager::BufferManager() {}

void BufferManager::WaitForBuffer(TypeFlags buffer) {

  switch (buffer) {
  case TypeFlags::BUFFER_DRAW_CALL_DATA:
    DrawCommandBuffer.WaitForBuffer();
    break;
  case TypeFlags::BUFFER_INSTANCE_DATA:
    InstanceData.WaitForBuffer();
    break;
  case TypeFlags::BUFFER_ANIMATION_DATA:
    AnimationMatrices.WaitForBuffer();
    break;
  case TypeFlags::BUFFER_PARTICLE_DATA:
    ParticleData.WaitForBuffer();
    break;
  case TypeFlags::BUFFER_TEXTURE_DATA:
    TextureHandleBuffer.WaitForBuffer();
    break;
  case TypeFlags::BUFFER_CAMERA_DATA:
    cameraMatrices.WaitForBuffer();
    break;
  case TypeFlags::BUFFER_LIGHT_DATA:
    LightsBuffer.WaitForBuffer();
    break;
  case TypeFlags::BUFFER_STATIC_MATRIX_DATA:
    StaticMatrices.WaitForBuffer();
    break;
  case TypeFlags::BUFFER_DEBUG_SHAPE_DATA_UINT:
    DebugShapeIndices.WaitForBuffer();
    break;
  case TypeFlags::BUFFER_DEBUG_DRAW_CALL_DATA:
    DebugShapeDrawCallData.WaitForBuffer();
    break;
  case TypeFlags::BUFFER_DEBUG_SHAPE_MATRIX_DATA:
    DebugShapeMatrices.WaitForBuffer();
    break;
  case TypeFlags::BUFFER_GI_PROBE_DATA:
    GI_ProbeBuffer.WaitForBuffer();
    break;
  case TypeFlags::BUFFER_GI_GRID_DATA:
    GI_GridBuffer.WaitForBuffer();
    break;
  default:
    SDL_Log("GetGLBufferID: Unknown buffer type %d\n", buffer);
  }
}
std::vector<GLuint> BufferManager::GetGLBufferID(TypeFlags Buffer) {
  std::vector<GLuint> buffer{0};

  switch (Buffer) {
  case TypeFlags::BUFFER_DRAW_CALL_DATA:
    buffer = DrawCommandBuffer.GetGLBufferID();
    break;
  case TypeFlags::BUFFER_INSTANCE_DATA:
    buffer = InstanceData.GetGLBufferID();
    break;
  case TypeFlags::BUFFER_ANIMATION_DATA:
    buffer = AnimationMatrices.GetGLBufferID();
    break;
  case TypeFlags::BUFFER_PARTICLE_DATA:
    buffer = ParticleData.GetGLBufferID();
    break;
  case TypeFlags::BUFFER_TEXTURE_DATA:
    buffer = TextureHandleBuffer.GetGLBufferID();
    break;
  case TypeFlags::BUFFER_CAMERA_DATA:
    buffer = cameraMatrices.GetGLBufferID();
    break;
  case TypeFlags::BUFFER_LIGHT_DATA:
    buffer = LightsBuffer.GetGLBufferID();
    break;
  case TypeFlags::BUFFER_STATIC_MATRIX_DATA:
    buffer = StaticMatrices.GetGLBufferID();
    break;
  case TypeFlags::BUFFER_DEBUG_SHAPE_DATA_UINT:
    buffer = DebugShapeIndices.GetGLBufferID();
    break;
  case TypeFlags::BUFFER_DEBUG_DRAW_CALL_DATA:
    buffer = DebugShapeDrawCallData.GetGLBufferID();
    break;
  case TypeFlags::BUFFER_DEBUG_SHAPE_MATRIX_DATA:
    buffer = DebugShapeMatrices.GetGLBufferID();
    break;
  case TypeFlags::BUFFER_GI_PROBE_DATA:
    buffer = GI_ProbeBuffer.GetGLBufferID();
    break;
  case TypeFlags::BUFFER_GI_GRID_DATA:
    buffer = GI_GridBuffer.GetGLBufferID();
    break;
  default:
    SDL_Log("GetGLBufferID: Unknown buffer type %d\n", Buffer);
    return buffer;
  }
  return buffer;
}
void BufferManager::BindDynamicBuffer(TypeFlags type) {
  CDynamicBuffer *buffer = nullptr;

  switch (type) {
  case TypeFlags::BUFFER_DRAW_CALL_DATA:
    buffer = &DrawCommandBuffer;
    break;
  case TypeFlags::BUFFER_INSTANCE_DATA:
    buffer = &InstanceData;
    break;
  case TypeFlags::BUFFER_ANIMATION_DATA:
    buffer = &AnimationMatrices;
    break;
  case TypeFlags::BUFFER_PARTICLE_DATA:
    buffer = &ParticleData;
    break;
  case TypeFlags::BUFFER_TEXTURE_DATA:
    buffer = &TextureHandleBuffer;
    break;
  case TypeFlags::BUFFER_CAMERA_DATA:
    buffer = &cameraMatrices;
    break;
  case TypeFlags::BUFFER_LIGHT_DATA:
    buffer = &LightsBuffer;
    break;
  case TypeFlags::BUFFER_STATIC_MATRIX_DATA:
    buffer = &StaticMatrices;
    break;
  case TypeFlags::BUFFER_DEBUG_DRAW_CALL_DATA:
    buffer = &DebugShapeDrawCallData;
    break;
  case TypeFlags::BUFFER_DEBUG_SHAPE_DATA_UINT:
    buffer = &DebugShapeIndices;
    break;
  case TypeFlags::BUFFER_DEBUG_SHAPE_MATRIX_DATA:
    buffer = &DebugShapeMatrices;
    break;
  case TypeFlags::BUFFER_GI_PROBE_DATA:
    buffer = &GI_ProbeBuffer;
    break;
  case TypeFlags::BUFFER_GI_GRID_DATA:
    buffer = &GI_GridBuffer;
    break;

  default:
    SDL_Log("BindDynamicBuffer: Unknown buffer type %d\n", type);
    return;
  }

  // Manager decides which slot to use
  buffer->SetSlot(buffer->GetWriteSlot());
}

void BufferManager::Initialize() {
  int d_size = 10;
  int s_size = 16;

  InstanceData = CDynamicBuffer(MBsize(d_size), 0);
  DrawCommandBuffer =
      CDynamicBuffer(MBsize(d_size), 1, GL_DRAW_INDIRECT_BUFFER);
  AnimationMatrices = CDynamicBuffer(MBsize(d_size), 2);
  TextureHandleBuffer = CDynamicBuffer(MBsize(d_size), 3);
  ParticleData = CDynamicBuffer(MBsize(d_size), 4);
  StaticMeshInformation = CGLStaticStack(MBsize(s_size), MBsize(s_size), 5);
  TerrainBuffer = CGLStaticStack(MBsize(s_size), MBsize(s_size), 6);

  // TODO: ADD the other static allocator

  // StaticMatrices = StaticBuffer(MBsize(s_size), MBsize(s_size), 7);
  cameraMatrices = CDynamicBuffer(MBsize(1), 8);
  LightsBuffer = CDynamicBuffer(MBsize(d_size), 9);
  StaticMatrices =
      CDynamicBuffer(MBsize(d_size), 10, GL_SHADER_STORAGE_BUFFER, false);

  DebugShapeDrawCallData =
      CDynamicBuffer(MBsize(5), 11, GL_DRAW_INDIRECT_BUFFER);

  DebugShapeIndices =
      CDynamicBuffer(MBsize(5), 12, GL_SHADER_STORAGE_BUFFER, false);
  DebugShapeMatrices =
      CDynamicBuffer(MBsize(10), 13, GL_SHADER_STORAGE_BUFFER, true);

  GI_ProbeBuffer =
      CDynamicBuffer(MBsize(20), 14, GL_SHADER_STORAGE_BUFFER, false);

  GI_GridBuffer =
      CDynamicBuffer(MBsize(20), 15, GL_SHADER_STORAGE_BUFFER, false);

  StaticbufferIDs.push_back(&StaticMeshInformation);
  StaticbufferIDs.push_back(&TerrainBuffer);

  DynamicBufferIDs.push_back(&InstanceData);
  DynamicBufferIDs.push_back(&AnimationMatrices);
  DynamicBufferIDs.push_back(&TextureHandleBuffer);
  DynamicBufferIDs.push_back(&ParticleData);
  DynamicBufferIDs.push_back(&DrawCommandBuffer);
  DynamicBufferIDs.push_back(&cameraMatrices);
  DynamicBufferIDs.push_back(&LightsBuffer);
  DynamicBufferIDs.push_back(&StaticMatrices);

  DynamicBufferIDs.push_back(&DebugShapeDrawCallData);
  DynamicBufferIDs.push_back(&DebugShapeIndices);
  DynamicBufferIDs.push_back(&DebugShapeMatrices);

  DynamicBufferIDs.push_back(&GI_ProbeBuffer);

  DynamicBufferIDs.push_back(&GI_GridBuffer);

  InstanceData.SetBinding(0);
  DrawCommandBuffer.SetBinding(1);
  AnimationMatrices.SetBinding(2);
  TextureHandleBuffer.SetBinding(3);
  ParticleData.SetBinding(4);
  cameraMatrices.SetBinding(5);
  LightsBuffer.SetBinding(6);
  StaticMatrices.SetBinding(7);

  DebugShapeDrawCallData.SetBinding(8);

  DebugShapeIndices.SetBinding(9);
  DebugShapeMatrices.SetBinding(10);

  GI_ProbeBuffer.SetBinding(11);

  GI_GridBuffer.SetBinding(12);
}
void BufferManager::BeginWritting() {
  for (auto &buffer : DynamicBufferIDs) {
    buffer->BeginWritting();
  }
}
VertexIndexInfoPair BufferManager::InsertNewStaticData(
    const Vertex *vertexData, size_t vertexDataSize, const GLuint *indexData,
    size_t indexDataSize, TypeFlags type = TypeFlags::BUFFER_STATIC_MESH_DATA) {
  // for now only use the StaticMeshInformation, later implement seperation

  // if (m_bUseStack) {

  if (type == TypeFlags::BUFFER_STATIC_MESH_DATA) // {
    return StaticMeshInformation.push_back(vertexData, vertexDataSize,
                                           indexData, indexDataSize);
  // }

  /*  if (type ==
        TypeFlags::
            BUFFER_STATIC_TERRAIN_DATA) { // NOTE: this is like this
                                          // because i dont know how to
                                          // design the RenderFrame() to
                                          // account for the different draw
                                          // indirect commands which are
                                          // mixed of course i could
                                          // seperate them, but this should
                                          // work for now. return
                                          //
  TerrainBuffer.InsertIntoBuffer(vertexData,
                                          // vertexDataSize, indexData,
                                          // indexDataSize);

      return StaticMeshInformation.push_back(vertexData, vertexDataSize,
                                             indexData, indexDataSize);
    }
  } else {
    // TODO: IMPLEMENT
  }                            */
  return VertexIndexInfoPair();
}
SBufferRange BufferManager::InsertNewDynamicData(const void *data, size_t size,
                                                 TypeFlags type) {

  if (type == TypeFlags::BUFFER_INSTANCE_DATA) {
    return InstanceData.InsertNewData(data, size, type);
  }
  if (type == TypeFlags::BUFFER_STATIC_MATRIX_DATA) {

    // SDL_Log("Staic data inserted\n");

    return StaticMatrices.InsertNewData(data, size, type);
  }
  if (type == TypeFlags::BUFFER_ANIMATION_DATA) {
    return AnimationMatrices.InsertNewData(data, size, type);
  }
  if (type == TypeFlags::BUFFER_PARTICLE_DATA) {
    return ParticleData.InsertNewData(data, size, type);
  }
  if (type == TypeFlags::BUFFER_TEXTURE_DATA) {
    return TextureHandleBuffer.InsertNewData(data, size, type);
  }
  if (type == TypeFlags::BUFFER_DRAW_CALL_DATA) {
    return DrawCommandBuffer.InsertNewData(data, size, type);
  }
  if (type == TypeFlags::BUFFER_CAMERA_DATA) {
    return cameraMatrices.InsertNewData(data, size, type);
  }
  if (type == TypeFlags::BUFFER_LIGHT_DATA) {
    return LightsBuffer.InsertNewData(data, size, type);
  }

  if (type == TypeFlags::BUFFER_DEBUG_SHAPE_DATA_UINT) {
    return DebugShapeIndices.InsertNewData(data, size, type);
  }
  if (type == TypeFlags::BUFFER_DEBUG_DRAW_CALL_DATA) {
    return DebugShapeDrawCallData.InsertNewData(data, size, type);
  }
  if (type == TypeFlags::BUFFER_DEBUG_SHAPE_MATRIX_DATA) {
    return DebugShapeMatrices.InsertNewData(data, size, type);
  }
  if (type == TypeFlags::BUFFER_GI_PROBE_DATA) {
    return GI_ProbeBuffer.InsertNewData(data, size, type);
  }
  if (type == TypeFlags::BUFFER_GI_GRID_DATA) {
    return GI_GridBuffer.InsertNewData(data, size, type);
  }

  SDL_Log("DYNAMIC BUFFER INSERTION ERROR: COULD NOT FIND THE DESIRED TYPE!\n");
  return SBufferRange();
}
void BufferManager::UpdateData(SBufferRange &range, const void *data,
                               const size_t size) {

  const uint16_t bufferID = range.handle.bufferID;

  for (CDynamicBuffer *buffer : DynamicBufferIDs) {
    if (buffer->GetBufferID() == bufferID) {
      buffer->UpdateRange(&range, data, size);
      return;
    }
  }

  SDL_Log("ERROR: BufferManager::UpdateData() - DynamicBuffer ID not found: %u",
          bufferID);
}

void BufferManager::ClearBuffer(TypeFlags whichBuffer) {

  if (whichBuffer == TypeFlags::BUFFER_STATIC_MESH_DATA) {

    if (m_bUseStack)
      StaticMeshInformation.Clear();
    else {
      // TODO: IMPLEMENT
    }
  }
  if (whichBuffer == TypeFlags::BUFFER_STATIC_TERRAIN_DATA) {
    // TerrainBuffer.ClearBuffer();

    if (m_bUseStack)
      StaticMeshInformation.Clear();
    else {
      // TODO: IMPLEMENT X2
    };
  }
  if (whichBuffer == TypeFlags::BUFFER_INSTANCE_DATA) {
    InstanceData.ClearBuffer();
  }
  if (whichBuffer == TypeFlags::BUFFER_MATRIX_DATA) {
    // guess we dont need it if we have instance data lmao
  }
  if (whichBuffer == TypeFlags::BUFFER_ANIMATION_DATA) {
    AnimationMatrices.ClearBuffer();
  }
  if (whichBuffer == TypeFlags::BUFFER_PARTICLE_DATA) {
    ParticleData.ClearBuffer();
  }
  if (whichBuffer == TypeFlags::BUFFER_TEXTURE_DATA) {
    TextureHandleBuffer.ClearBuffer();
  }
  if (whichBuffer == TypeFlags::BUFFER_DRAW_CALL_DATA) {
    DrawCommandBuffer.ClearBuffer();
  }
  if (whichBuffer == TypeFlags::BUFFER_STATIC_MATRIX_DATA) {

    StaticMatrices.ClearBuffer();
  }
  if (whichBuffer == TypeFlags::BUFFER_CAMERA_DATA) {
    cameraMatrices.ClearBuffer();
  }
  if (whichBuffer == TypeFlags::BUFFER_LIGHT_DATA) {
    LightsBuffer.ClearBuffer();
  }
  if (whichBuffer == TypeFlags::BUFFER_DEBUG_DRAW_CALL_DATA ||
      whichBuffer == TypeFlags::BUFFER_DEBUG_SHAPE_DATA_UINT) {
    DebugShapeDrawCallData.ClearBuffer();
    DebugShapeIndices.ClearBuffer();
  }
  if (whichBuffer == TypeFlags::BUFFER_DEBUG_SHAPE_MATRIX_DATA) {
    DebugShapeMatrices.ClearBuffer();
  }
  if (whichBuffer == TypeFlags::BUFFER_GI_PROBE_DATA) {
    GI_ProbeBuffer.ClearBuffer();
  }
  if (whichBuffer == TypeFlags::BUFFER_GI_GRID_DATA) {
    GI_GridBuffer.ClearBuffer();
  }
}

void BufferManager::Destroy() {
  for (auto &buffer : StaticbufferIDs) {
    buffer->Destroy();
  }
  for (auto &buffer : DynamicBufferIDs) {
    buffer->Destroy();
  }
}
void BufferManager::EndWritting() {
  for (auto &buffer : DynamicBufferIDs) {
    buffer->EndWritting();
  }
}
void BufferManager::UpdateManager() {}

} // namespace eHazGraphics
