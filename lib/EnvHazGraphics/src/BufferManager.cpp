#include "BufferManager.hpp"
#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "StaticStack.hpp"
#include "glad/glad.h"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_stdinc.h>
#include <algorithm>
//#include <bits/types/mbstate_t.h>
#include <cassert>

#include <cstddef>

#include <cstdint>
#include <cstring>
#include <iterator>

#include <utility>
#include <vector>

namespace eHazGraphics {

BufferManager::BufferManager() {}





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
  default:
    SDL_Log("BindDynamicBuffer: Unknown buffer type %d\n", type);
    return;
  }

  // Manager decides which slot to use
  buffer->SetSlot(buffer->GetWriteSlot());
}

/*DynamicBuffer::DynamicBuffer(size_t initialBuffersSize, int DynamicBufferID,
                             GLenum target, bool trippleBuffer)
    : DynamicBufferID(DynamicBufferID), Target(target),
      trippleBuffer(trippleBuffer) {
  // TODO: make it so that if we have trippleBuffer be true, only create 1
  // buffer
  slotFullSize[0] = initialBuffersSize;
  slotFullSize[1] = initialBuffersSize;
  slotFullSize[2] = initialBuffersSize;
  glCreateBuffers(1, &BufferSlots[0]);
  glCreateBuffers(1, &BufferSlots[1]);
  glCreateBuffers(1, &BufferSlots[2]);

  glNamedBufferStorage(BufferSlots[0], initialBuffersSize, nullptr,
                       GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT |
                           GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

  glNamedBufferStorage(BufferSlots[1], initialBuffersSize, nullptr,
                       GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT |
                           GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
  glNamedBufferStorage(BufferSlots[2], initialBuffersSize, nullptr,
                       GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT |
                           GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

  MapAllBufferSlots();

#ifdef EHAZ_DEBUG
  GLint result = 0;
  glGetNamedBufferParameteriv(BufferSlots[0], GL_BUFFER_SIZE, &result);
  SDL_Log("Buffer %u size = %d", BufferSlots[0], result);

  GLint result1 = 0;
  glGetNamedBufferParameteriv(BufferSlots[1], GL_BUFFER_SIZE, &result1);
  SDL_Log("Buffer %u size = %d", BufferSlots[1], result1);

  GLint result2 = 0;
  glGetNamedBufferParameteriv(BufferSlots[2], GL_BUFFER_SIZE, &result2);
  SDL_Log("Buffer %u size = %d", BufferSlots[2], result2);
#endif
}*/

/*void DynamicBuffer::Destroy() {

  for (int i = 0; i < 3; ++i) {
    if (slots[i]) {
      // unmap if mapped
      glUnmapNamedBuffer(BufferSlots[i]);
      slots[i] = nullptr;
    }
    if (fences[i]) {
      glDeleteSync(fences[i]);
      fences[i] = 0;
    }
    if (glIsBuffer(BufferSlots[i])) {
      glDeleteBuffers(1, &BufferSlots[i]);
    }
    BufferSlots[i] = 0;
  }
}*/

/*void DynamicBuffer::SetSlot(int slot) {
  if (trippleBuffer)
    currentSlot = slot;
  else
    currentSlot = 0;

  if (!glIsBuffer(BufferSlots[slot])) {
#ifndef EHAZ_DEBUG
    SDL_Log("SetSlot(): BufferSlots[%d]=%u is not a valid buffer", slot,
            BufferSlots[slot]);
#endif
    return;
  }

  switch (Target) {
  case GL_SHADER_STORAGE_BUFFER:
  case GL_UNIFORM_BUFFER:
  case GL_ATOMIC_COUNTER_BUFFER:
  case GL_TRANSFORM_FEEDBACK_BUFFER:
    // Indexed binding targets

    glBindBufferBase(Target, binding, BufferSlots[currentSlot]);

  case GL_DRAW_INDIRECT_BUFFER:
  case GL_ARRAY_BUFFER:
  case GL_ELEMENT_ARRAY_BUFFER:
    // Non-indexed binding targets
    glBindBuffer(Target, BufferSlots[currentSlot]);
    break;

  default:
    SDL_Log("SetSlot(): Unsupported Target=0x%X", Target);
    break;
  }

#ifdef EHAZ_DEBUG
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    SDL_Log("SetSlot(): bind failed for target=0x%X slot=%d buffer=%u err=0x%X",
            Target, currentSlot, BufferSlots[currentSlot], err);
  }
#endif
}*/

/*void DynamicBuffer::SetSlot(int slot)
{
    currentSlot = slot;

    glBindBufferBase(Target, binding, BufferSlots[slot]);
}*/
//void DynamicBuffer::SetBinding(int bindingNum) { binding = bindingNum; }

// buffer resize logic:

/*void DynamicBuffer::BeginWritting() {
  // Select a free slot (triple buffering) or stall until one becomes free
  
    //allocations.clear();
    //freeAllocationIDs.clear();
    if (trippleBuffer) {
    bool found = false;
    for (int i = 0; i < 3; ++i) {
      int candidate = (currentSlot + i + 1) % 3;
      if (waitForSlotFence(candidate)) {
        nextSlot = candidate;
        found = true;
        break;
      }
    }

    if (!found) {
      // No free slot found, force wait on the oldest slot
      auto max = std::max_element(slotsAge, slotsAge + 3);
      int oldest = std::distance(slotsAge, max);

      GLenum waitRes = glClientWaitSync(
          fences[oldest], GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(1e9));
      if (waitRes == GL_TIMEOUT_EXPIRED)
        SDL_Log(
            "Warning: GPU still not finished with buffer slot %d, forced wait",
            oldest);

      glDeleteSync(fences[oldest]);
      fences[oldest] = 0;
      slotsAge[oldest] = 0;

      nextSlot = oldest;
    }
  } else {
    nextSlot = 0;
    currentSlot = 0;
  }
}

void DynamicBuffer::ReCreateBuffer(size_t minimumSize) {
  for (unsigned int i = 0; i < 3; ++i) {
    if (!shouldResize[i])
      continue;

    if (!waitForSlotFence(i))
      continue;

    GLuint newBuffer = 0;
#ifdef PLATFORM_WINDOWS
    size_t newSize = max(2 * slotFullSize[i], minimumSize);
#elif defined(PLATFORM_LINUX)
    size_t newSize = std::max(2 * slotFullSize[i], minimumSize);
#endif // PLATFORM_WINDOWS

    
    if (newSize == 0)
      newSize = 1024; // sanity fallback

    glCreateBuffers(1, &newBuffer);
    glNamedBufferStorage(newBuffer, newSize, nullptr,
                         GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT |
                             GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

    GLint result = 0;
    glGetNamedBufferParameteriv(newBuffer, GL_BUFFER_SIZE, &result);
    SDL_Log("Buffer %u size = %d needed size: %zu", newBuffer, result, newSize);

    if (result == 0) {
      SDL_Log("ReCreateBuffer: allocation failed for slot %u (newBuffer=%u)", i,
              newBuffer);
      glDeleteBuffers(1, &newBuffer);
      continue;
    }

    // Copy old data if valid
    if (glIsBuffer(BufferSlots[i]) && slotOccupiedSize[i] > 0) {
      glUnmapNamedBuffer(BufferSlots[i]); // safe even if not mapped
      slots[i] = nullptr;

      glCopyNamedBufferSubData(BufferSlots[i], newBuffer, 0, 0,
                               slotOccupiedSize[i]);
      GLenum err = glGetError();
      if (err != GL_NO_ERROR) {
        SDL_Log("ReCreateBuffer: copy failed slot=%u old=%u new=%u err=0x%X", i,
                BufferSlots[i], newBuffer, err);
        glDeleteBuffers(1, &newBuffer);
        continue;
      }
    }

    // Map new buffer

    slots[i] = glMapNamedBufferRange(newBuffer, 0, newSize,
                                     GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |
                                         GL_MAP_COHERENT_BIT);
    GLenum err = glGetError();
    if (!slots[i] || err != GL_NO_ERROR) {
      SDL_Log("ReCreateBuffer: map failed newBuffer=%u slot=%u err=0x%X",
              newBuffer, i, err);
      glDeleteBuffers(1, &newBuffer);
      slots[i] = nullptr;
      continue;
    }

    // Success: swap in new buffer
    if (glIsBuffer(BufferSlots[i]))
      glDeleteBuffers(1, &BufferSlots[i]);

    BufferSlots[i] = newBuffer;
    slotFullSize[i] = newSize;
    shouldResize[i] = false;

    SDL_Log(
        "ReCreateBuffer: slot %u replaced with buffer %u size=%zu (used=%zu)",
        i, BufferSlots[i], newSize, slotOccupiedSize[i]);
  }
}

void DynamicBuffer::MapAllBufferSlots() {
  for (unsigned int i = 0; i < 3; i++) {

    if (glIsBuffer(BufferSlots[i])) {

      slots[i] = glMapNamedBufferRange(
          BufferSlots[i], 0, slotFullSize[i],
          GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    } else {
      shouldResize[i] = true;
    }
  }
}*/

/*SBufferRange DynamicBuffer::InsertNewData(const void *data, size_t size,
                                          TypeFlags type) {
  // for (unsigned int i = 0; i < 3; i++)
  //{
  unsigned int i = nextSlot;
  if (slotFullSize[i] >= size + slotOccupiedSize[i]) // allow exact fit
  {

    if (slots[i] != nullptr) {
      // compute byte-based insertion location
      std::byte *base =
          static_cast<std::byte *>(slots[i]) + slotOccupiedSize[i];
      int count = 1;
      switch (type) {
      case TypeFlags::BUFFER_INSTANCE_DATA: {
        auto *typedPtr = reinterpret_cast<InstanceData *>(base);
        count = size / sizeof(InstanceData);

        std::memcpy(typedPtr, data, size);
        break;
      }
      case TypeFlags::BUFFER_CAMERA_DATA: {
        auto *typedPtr = reinterpret_cast<glm::mat4 *>(base);
        count = size / sizeof(glm::mat4);
        std::memcpy(typedPtr, data, size);
        break;
      }
      case TypeFlags::BUFFER_ANIMATION_DATA: {
        auto *typedPtr = reinterpret_cast<glm::mat4 *>(base);
        count = size / sizeof(glm::mat4);
        std::memcpy(typedPtr, data, size);
        break;
      }
      case TypeFlags::BUFFER_DRAW_CALL_DATA: {
        auto *typedPtr = reinterpret_cast<DrawElementsIndirectCommand *>(base);
        count = size / sizeof(DrawElementsIndirectCommand);
        std::memcpy(typedPtr, data, size);
        break;
      }
      case TypeFlags::BUFFER_TEXTURE_DATA: {
        auto *typedPtr = reinterpret_cast<GLuint64 *>(base); // bindless handle
        count = size / sizeof(GLuint64);
        std::memcpy(typedPtr, data, size);
        break;
      }
      case TypeFlags::BUFFER_LIGHT_DATA: {
        // TODO: implement LightData properly
        std::memcpy(base, data, size);
        break;
      }
      case TypeFlags::BUFFER_PARTICLE_DATA: {
        // TODO: implement ParticleData properly
        std::memcpy(base, data, size);
        break;
      }
      case TypeFlags::BUFFER_STATIC_MATRIX_DATA: {

        auto *typedPtr = reinterpret_cast<glm::mat4 *>(base);
        count = size / sizeof(glm::mat4);
        std::memcpy(typedPtr, data, size);

        break;
      }

      default: {
        SDL_Log("InsertNewData: unknown buffer type %d",
                static_cast<int>(type));
        std::memcpy(base, data, size); // fallback: raw copy
        break;
      }
      }

      // prepare return range

      SBufferRange RT;

      RT.OwningBuffer = DynamicBufferID;
      RT.slot = i;
      RT.offset = slotOccupiedSize[i];
      RT.size = size;
      RT.count =
          count; // forgot what this was for so its just the count of items
      RT.dataType = type;

      slotOccupiedSize[i] += size; // advance by bytes

      return RT;
    } else {
      SDL_Log("InsertNewData: slots[%u] of buffer %u is not mapped, attempting "
              "recovery",
              i, DynamicBufferID);
      ReCreateBuffer(size);
      if (slots[i]) // retry once
        return InsertNewData(data, size, type);
      // continue;
    }
  } else {
    SDL_Log("InsertNewData: slot %u too small, marking resize", i);
    shouldResize[i] = true;
  }
  //   }

  // If we get here, no slot worked → force resize and retry
  SDL_Log("InsertNewData: no fitting slot found, forcing resize");
  ReCreateBuffer(size);
  return InsertNewData(data, size, type);
} */

/*SBufferRange DynamicBuffer::InsertNewData(const void* data, size_t size,
                                          TypeFlags type) {
  unsigned int i = nextSlot;

  // Ensure there is enough room in the current slot
  if (slotFullSize[i] < size + slotOccupiedSize[i]) {
    SDL_Log("InsertNewData: slot %u too small, marking resize", i);
    shouldResize[i] = true;
    ReCreateBuffer(size);
    return InsertNewData(data, size, type); // retry after resize
  }

  if (!slots[i]) {
    SDL_Log("InsertNewData: slots[%u] of buffer %u is not mapped, attempting "
            "recovery",
            i, DynamicBufferID);
    ReCreateBuffer(size);
    if (!slots[i]) {
      SDL_Log("InsertNewData: failed to recover buffer slot %u", i);
      return SBufferRange{};
    }
  }

  // Compute byte-based insertion point
  std::byte *base = static_cast<std::byte *>(slots[i]) + slotOccupiedSize[i];
  uint32_t count = 1;

  // Copy data according to type
  switch (type) {
  case TypeFlags::BUFFER_INSTANCE_DATA:
    count = size / sizeof(InstanceData);
    std::memcpy(reinterpret_cast<InstanceData *>(base), data, size);
    break;
  case TypeFlags::BUFFER_CAMERA_DATA:
  case TypeFlags::BUFFER_ANIMATION_DATA:
  case TypeFlags::BUFFER_STATIC_MATRIX_DATA:
    count = size / sizeof(glm::mat4);
    std::memcpy(reinterpret_cast<glm::mat4 *>(base), data, size);
    break;
  case TypeFlags::BUFFER_DRAW_CALL_DATA:
    count = size / sizeof(DrawElementsIndirectCommand);
    std::memcpy(reinterpret_cast<DrawElementsIndirectCommand *>(base), data,
                size);
    break;
  case TypeFlags::BUFFER_TEXTURE_DATA:
    count = size / sizeof(GLuint64);
    std::memcpy(reinterpret_cast<GLuint64 *>(base), data, size);
    break;
  case TypeFlags::BUFFER_LIGHT_DATA:
  case TypeFlags::BUFFER_PARTICLE_DATA:
  default:
    std::memcpy(base, data, size);
    break;
  }

  // Allocate a new SAllocation entry
  uint32_t allocID = AllocateID(size);
  SAllocation &alloc = allocations[allocID];
  //  alloc.offset = slotOccupiedSize[i]; // byte offset
  //  alloc.size = size;
  //  alloc.alive = true;
  //  alloc.generation++;

  // Prepare the buffer handle
  SBufferHandle handle{.bufferID = static_cast<uint16_t>(DynamicBufferID),
                       .slot = GetDynamicSlotType(i),
                       .allocationID = allocID,
                       .generation = alloc.generation};

  // Advance the occupied size
  slotOccupiedSize[i] += size;

  // Prepare and return the SBufferRange
  SBufferRange range{.handle = handle, .dataType = type, .count = count};

  return range;
}

// ---------------------- PATCH END -----------------------

// =====================================END IF PATCH==============================================\\


void DynamicBuffer::UpdateOldData(SBufferRange range, const void *data,
                                  size_t size) {
  const SBufferHandle &handle = range.handle;

  // Validate allocation ID
  if (handle.allocationID >= allocations.size()) {
    SDL_Log("UpdateOldData: invalid allocationID %u for buffer %u",
            handle.allocationID, handle.bufferID);
    assert(false);
    return;
  }

  SAllocation &alloc = allocations[handle.allocationID];

  // Check generation
  if (alloc.generation != handle.generation || !alloc.alive) {
    SDL_Log("UpdateOldData: allocation is stale or deleted (allocID=%u, "
            "generation=%u)",
            handle.allocationID, handle.generation);
    assert(false);
    return;
  }

  // Validate slot mapping
  int slotIndex = nextSlot;
  /*for (int s = 0; s < 3; ++s) {
    if (BufferSlots[s] == handle.slot) {
      slotIndex = s;
      break;
    }
  }*/
  /*if (slotIndex == -1 || !slots[slotIndex]) {
    SDL_Log("UpdateOldData: slot %u not mapped or invalid", handle.slot);
    assert(false);
    return;
  }

  // Compute byte offset
  std::byte *base = static_cast<std::byte *>(slots[slotIndex]) + alloc.offset;

  // Copy data according to type
  switch (range.dataType) {
  case TypeFlags::BUFFER_INSTANCE_DATA:
    std::memcpy(reinterpret_cast<InstanceData *>(base), data, size);
    break;
  case TypeFlags::BUFFER_CAMERA_DATA:
  case TypeFlags::BUFFER_ANIMATION_DATA:
  case TypeFlags::BUFFER_STATIC_MATRIX_DATA:
    std::memcpy(reinterpret_cast<glm::mat4 *>(base), data, size);
    break;
  case TypeFlags::BUFFER_DRAW_CALL_DATA:
    std::memcpy(reinterpret_cast<DrawElementsIndirectCommand *>(base), data,
                size);
    break;
  case TypeFlags::BUFFER_TEXTURE_DATA:
    std::memcpy(reinterpret_cast<GLuint64 *>(base), data, size);
    break;
  case TypeFlags::BUFFER_LIGHT_DATA:
  case TypeFlags::BUFFER_PARTICLE_DATA:
  default:
    std::memcpy(base, data, size);
    break;
  }
}*/

/*void DynamicBuffer::UpdateOldData(SBufferRange range, const void *data,
                                  size_t size) {

  TypeFlags type = range.dataType;

  if (BufferSlots[nextSlot] == 0 || !glIsBuffer(BufferSlots[nextSlot])) {
    SDL_Log("ERROR: trying to use invalid buffer ID %u at slot %d ; called "
            "from DynamicBuffer::UpdateOldData()",
            BufferSlots[range.OwningBuffer], range.slot);
    assert(false);
  }
  if (!slots[currentSlot]) {
    SDL_Log("ERROR: slots[%d] pointer is null even though buffer id %u exists "
            ";  called from "
            "DynamicBuffer::UpdateOldData()",
            range.slot, BufferSlots[nextSlot]);
    assert(false);
  }

  std::byte *base = static_cast<std::byte *>(slots[nextSlot]) + range.offset;

  switch (type) {
  case TypeFlags::BUFFER_INSTANCE_DATA: {
    auto *typedPtr = reinterpret_cast<InstanceData *>(base);
    std::memcpy(typedPtr, data, size);
    break;
  }
  case TypeFlags::BUFFER_CAMERA_DATA: {
    auto *typedPtr = reinterpret_cast<glm::mat4 *>(base);
    std::memcpy(typedPtr, data, size);
    break;
  }
  case TypeFlags::BUFFER_DRAW_CALL_DATA: {
    auto *typedPtr = reinterpret_cast<DrawElementsIndirectCommand *>(base);
    std::memcpy(typedPtr, data, size);
    break;
  }
  case TypeFlags::BUFFER_TEXTURE_DATA: {
    auto *typedPtr = reinterpret_cast<GLuint64 *>(base);
    std::memcpy(typedPtr, data, size);
    break;
  }
  case TypeFlags::BUFFER_ANIMATION_DATA: {
    auto *typedPtr = reinterpret_cast<glm::mat4 *>(base);
    std::memcpy(typedPtr, data, size);
    break;
  }
  default:
    std::memcpy(base, data, size); // fallback
    break;
  }
}*/

/*void DynamicBuffer::EndWritting()
{
    glBindBuffer(Target, BufferSlots[nextSlot]);

    // Place a fence to track GPU completion for this slot

    if (trippleBuffer)
    {
        SetDownFence();

        // Increase timeline so we can track slot ages
        slotsAge[nextSlot] = ++slotTimeline;

        // Commit swap: nextSlot becomes the current slot used for rendering
        currentSlot = nextSlot;
    }
}*/

/*void DynamicBuffer::EndWritting() {
  // Bind the slot that will be used for rendering
  glBindBuffer(Target, BufferSlots[nextSlot]);

  // Place fence for the slot we just wrote (nextSlot)
  // We set fence for nextSlot explicitly so waitForSlotFence checks the right
  // sync object.
  SetDownFence(nextSlot);

  // Commit swap: nextSlot becomes current slot used for rendering
  currentSlot = nextSlot;

  // Note: slot age already set by SetDownFence
}*/

// privates:

/*bool DynamicBuffer::waitForSlotFence(int slot) {
  if (fences[slot] == 0)
    return true;

  // Flush commands to GPU before waiting
  GLenum res = glClientWaitSync(fences[slot], GL_SYNC_FLUSH_COMMANDS_BIT, 0);

  if (res == GL_ALREADY_SIGNALED || res == GL_CONDITION_SATISFIED) {
    glDeleteSync(fences[slot]);
    fences[slot] = 0;
    return true;
  }

  return false; // slot still busy
}*/

/*void DynamicBuffer::SetDownFence()
{
    if (fences[currentSlot])
    {
        glDeleteSync(fences[currentSlot]);
    }
    fences[currentSlot] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    slotsAge[currentSlot] = slotTimeline;
    slotTimeline++;

}*/

// in class declaration: void SetDownFence(int slot);
/*void DynamicBuffer::SetDownFence(int slot) {
  if (slot < 0 || slot >= 3)
    return;

  if (fences[slot]) {
    glDeleteSync(fences[slot]);
  }
  fences[slot] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

  // update age for the slot we just fenced
  slotsAge[slot] = ++slotTimeline;
}*/

//-----------------------DYNAMIC BUFFER IMPLEMENTATION END-------------------------------------\\


void BufferManager::Initialize() {
  int d_size = 10;
  int s_size = 16;

  InstanceData = CDynamicBuffer(MBsize(d_size), 0);
  DrawCommandBuffer = CDynamicBuffer(MBsize(d_size), 1, GL_DRAW_INDIRECT_BUFFER);
  AnimationMatrices = CDynamicBuffer(MBsize(d_size), 2);
  TextureHandleBuffer = CDynamicBuffer(MBsize(d_size), 3);
  ParticleData = CDynamicBuffer(MBsize(d_size), 4);
  StaticMeshInformation = CGLStaticStack(MBsize(s_size), MBsize(s_size), 5);
  TerrainBuffer = CGLStaticStack(MBsize(s_size), MBsize(s_size), 6);

  // TODO: ADD the other static allocator

  // StaticMatrices = StaticBuffer(MBsize(s_size), MBsize(s_size), 7);
  cameraMatrices = CDynamicBuffer(2 * sizeof(glm::mat4), 8);
  LightsBuffer = CDynamicBuffer(MBsize(d_size), 9);
  StaticMatrices =
      CDynamicBuffer(MBsize(d_size), 10, GL_SHADER_STORAGE_BUFFER, false);

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

  InstanceData.SetBinding(0);
  DrawCommandBuffer.SetBinding(1);
  AnimationMatrices.SetBinding(2);
  TextureHandleBuffer.SetBinding(3);
  ParticleData.SetBinding(4);
  cameraMatrices.SetBinding(5);
  LightsBuffer.SetBinding(6);
  StaticMatrices.SetBinding(7);
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

  if (m_bUseStack) {

    if (type == TypeFlags::BUFFER_STATIC_MESH_DATA) {
      return StaticMeshInformation.push_back(vertexData, vertexDataSize,
                                             indexData, indexDataSize);
    }

    if (type ==
        TypeFlags::
            BUFFER_STATIC_TERRAIN_DATA) { // NOTE: this is like this
                                          // because i dont know how to
                                          // design the RenderFrame() to
                                          // account for the different draw
                                          // indirect commands which are
                                          // mixed of course i could
                                          // seperate them, but this should
                                          // work for now. return
                                          // TerrainBuffer.InsertIntoBuffer(vertexData,
                                          // vertexDataSize, indexData,
                                          // indexDataSize);

      return StaticMeshInformation.push_back(vertexData, vertexDataSize,
                                             indexData, indexDataSize);
    }
  } else {
    // TODO: IMPLEMENT
  }
  return VertexIndexInfoPair();
}
SBufferRange BufferManager::InsertNewDynamicData(const void *data, size_t size,
                                                 TypeFlags type) {

  if (type == TypeFlags::BUFFER_INSTANCE_DATA) {
    return InstanceData.InsertNewData(data, size, type);
  }
  if (type == TypeFlags::BUFFER_STATIC_MATRIX_DATA) {
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
  if (whichBuffer == TypeFlags::BUFFER_CAMERA_DATA) {
    cameraMatrices.ClearBuffer();
  }
  if (whichBuffer == TypeFlags::BUFFER_LIGHT_DATA) {
    LightsBuffer.ClearBuffer();
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
