#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP

#include "BitFlags.hpp"
#include <DataStructs.hpp>
#include <StaticBuffer.hpp>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <algorithm>
#include <alloca.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <glad/glad.h>

#include <optional>
#include <ratio>
#include <regex>
#include <vector>

#include "DataStructs.hpp"

namespace eHazGraphics {

class DynamicBuffer {

public:
  DynamicBuffer();

  DynamicBuffer(size_t initialBuffersSize, int DynamicBufferID,
                GLenum target = GL_SHADER_STORAGE_BUFFER,
                bool trippleBuffer = true);

  void SetSlot(int slot);

  void SetBinding(int bindingNum);

  void ReCreateBuffer(size_t minimumSize = 1024UL);

  void ClearBuffer() {

    if (trippleBuffer) {
      for (int i = 0; i < 3; i++) {

        waitForSlotFence(i);
        float val = 0.0f;
        // glClearNamedBufferData(BufferSlots[i], GL_R32F, GL_RED, GL_FLOAT,
        // &val);
        //  SDL_memset(slots[i], 0, slotFullSize[i]);
      }
    } else {
      // glNamedBufferSubData(BufferSlots[0], 0, slotFullSize[0], nullptr);
    }

    allocations.clear();
    freeAllocationIDs.clear();
    slotOccupiedSize[0] = 0;
    slotOccupiedSize[1] = 0;
    slotOccupiedSize[2] = 0;
  }

  const SAllocation &GetAllocationData(uint32_t ID) { return allocations[ID]; }
  void BeginWritting();

  SBufferRange InsertNewData(const void *data, size_t size, TypeFlags type);

  void UpdateOldData(SBufferRange range, const void *data, size_t size);

  int GetDynamicBufferID() const { return DynamicBufferID; }
  int GetNextSlot() const { return nextSlot; }

  uint GetCurrentSlot() { return currentSlot; }

  void EndWritting();

  void Destroy();
  void RemoveItem(SBufferRange range) {

    if (slotFullSize[0] == slotFullSize[1] == slotFullSize[2]) {

      for (int i = 0; i < 3; i++) {
        slotOccupiedSize[i] -= allocations[range.handle.allocationID].size;
      }

    } else {

      slotOccupiedSize[GetDynamicSlotID(range.handle.slot)] -=
          allocations[range.handle.allocationID].size;
    }

    freeAllocationIDs.push_back(range.handle.allocationID);
  }
  ~DynamicBuffer() {
    // Destroy();
  }

private:
  SlotType GetDynamicSlotType(int ID) {
    switch (ID) {
    case 0:
      return SlotType::DYNAMIC_SLOT_1;
      break;
    case 1:
      return SlotType::DYNAMIC_SLOT_2;
      break;
    case 2:
      return SlotType::DYNAMIC_SLOT_3;
      break;
    default:
      return SlotType::SLOT_NONE;
      break;
    }
  }

  int GetDynamicSlotID(SlotType p_type) {
    switch (p_type) {

    case SlotType::DYNAMIC_SLOT_1:
      return 0;
      break;

    case SlotType::DYNAMIC_SLOT_2:
      return 1;
      break;
    case SlotType::DYNAMIC_SLOT_3:
      return 2;
      break;
    case SlotType::SLOT_NONE:
      return -1;
    }
  }

  uint32_t AllocateID(size_t size) {
    if (freeAllocationIDs.size() > 0) {

      for (int i = 0; i < freeAllocationIDs.size(); i++) {

        uint32_t l_checkID = freeAllocationIDs[i];
        if (allocations[l_checkID].size > size) {

          allocations.push_back(SAllocation());
          uint32_t l_remainingID = allocations.size();

          SAllocation &l_remainingAllocation = allocations[l_remainingID];

          l_remainingAllocation.alive = false;
          l_remainingAllocation.generation += 1;
          l_remainingAllocation.offset = allocations[l_checkID].offset + size;
          l_remainingAllocation.size =
              allocations[l_checkID].size - allocations[l_remainingID].size;

          return l_checkID;
        } else if (allocations[l_checkID].size == size) {
          return l_checkID;
        } else {
          continue;
        }
      }

    } else {

      SAllocation newAlloc = {.offset = slotOccupiedSize[nextSlot],
                              .size = size,
                              .alive = true,
                              .generation = newAlloc.generation + 1};

      allocations.push_back(newAlloc);

      return allocations.size() - 1;
    }
  }

  std::vector<SAllocation> allocations;
  std::vector<uint32_t> freeAllocationIDs;

  GLenum Target = GL_SHADER_STORAGE_BUFFER;
  int DynamicBufferID = 0;
  size_t slotFullSize[3]{0, 0, 0};
  size_t slotOccupiedSize[3]{0, 0, 0};
  // create the initial 3 arrays
  GLuint BufferSlots[3]{0, 0, 0};
  void *slots[3]{nullptr, nullptr, nullptr};
  int binding = 0;
  int currentSlot = 0;
  int nextSlot = 0;
  // add 3 fence variables + a function to initiate it
  GLsync fences[3]{0, 0, 0};
  bool shouldResize[3]{false, false, false};

  bool trippleBuffer = true;

  uint64_t slotTimeline = 0;
  uint64_t slotsAge[3]{0, 0, 0};

  void SetDownFence(int slot);
  void MapAllBufferSlots();
  bool waitForSlotFence(int slot);
};

class BufferManager {

public:
  BufferManager();

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
  }

  void RemoveStaicRange(VertexIndexInfoPair range) {
    for (auto &buffer : StaticbufferIDs) {
      if (buffer->GetStaticBufferID() == range.first.handle.bufferID) {
        buffer->RemoveItem(range);
      }
    }
  }

  void RemoveRange(SBufferRange range) {

    for (auto &buffer : DynamicBufferIDs) {
      if (buffer->GetDynamicBufferID() == range.handle.bufferID) {
        buffer->RemoveItem(range);
      }
    }
  }
  void UpdateData(const SBufferRange &range, const void *data,
                  const size_t size);

  const SAllocation &GetAllocation(const SBufferRange &range) {

    for (auto &buffer : StaticbufferIDs) {

      if (buffer->GetStaticBufferID() == range.handle.bufferID) {

        return buffer->GetAllocation(range);
      }
    }

    for (auto &buffer : DynamicBufferIDs) {

      if (buffer->GetDynamicBufferID() == range.handle.bufferID) {

        return buffer->GetAllocationData(range.handle.allocationID);
      }
    }
  }

  void EndWritting();

  void UpdateManager();

  void Destroy();

private:
  DynamicBuffer InstanceData;
  DynamicBuffer AnimationMatrices;
  DynamicBuffer TextureHandleBuffer;
  DynamicBuffer ParticleData;
  DynamicBuffer DrawCommandBuffer;
  DynamicBuffer cameraMatrices;
  DynamicBuffer LightsBuffer;
  DynamicBuffer StaticMatrices;

  CStaticBuffer StaticMeshInformation;
  CStaticBuffer TerrainBuffer;
  // StaticBuffer StaticMatrices;
  //  Every time a buffer is added update the following functions:
  //  Initialize(), InsertNew*Data() , ClearBuffer() and BitFlags

  // NOTE: ALSO FUCKING BIND THE SHIT IN THE DRAW CALL GOD DAMN.

  // CHANGE THESE EVERYTIME YOU ADD A BUFFER!!!!!!!!!
  unsigned int numOfDynamicBuffers = 8;
  unsigned int numofStaticBuffers = 2;

  std::vector<CStaticBuffer *> StaticbufferIDs;
  std::vector<DynamicBuffer *> DynamicBufferIDs;

  // std::unordered_map<MeshID, >
};

} // namespace eHazGraphics

#endif
