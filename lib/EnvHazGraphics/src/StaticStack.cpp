
#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "glad/glad.h"
#include <SDL3/SDL_log.h>
#include <StaticStack.hpp>
#include <algorithm>
#include <cassert>
#include <optional>

namespace eHazGraphics {

CGLStaticStack::CGLStaticStack() {}
CGLStaticStack::CGLStaticStack(size_t p_szInitialVertSize,
                               size_t p_szInitialIndexSize,
                               uint32_t p_StaticStackID)
    : m_szIndexBufferSize(p_szInitialIndexSize),
      m_szVertexBufferSize(p_szInitialVertSize),
      m_StaticStackID(p_StaticStackID)

{
  m_szVertexOccupiedSize = 0;
  m_szIndexOccupiedSize = 0;
  glCreateBuffers(1, &m_glVertexBuffer);
  glCreateBuffers(1, &m_glIndexBuffer);
  glCreateVertexArrays(1, &m_glVertexArray);

  glNamedBufferData(m_glVertexBuffer, p_szInitialVertSize, nullptr,
                    GL_DYNAMIC_DRAW);
  glNamedBufferData(m_glIndexBuffer, p_szInitialIndexSize, nullptr,
                    GL_DYNAMIC_DRAW);

  SetVertexAttribPointers();
}
std::optional<SAllocation>
CGLStaticStack::GetAllocation(const SBufferRange &p_range) const {

  if (isRangeValid(p_range)) {

    switch (p_range.handle.slot) {

    case SlotType::VERTEX_SLOT: {

      return m_VertexAllocations[p_range.handle.allocationID];

    } break;

    case SlotType::INDEX_SLOT: {
      return m_IndexAllcoations[p_range.handle.allocationID];
    } break;
    }
  }

  return std::nullopt;
}
VertexIndexInfoPair CGLStaticStack::push_back(const Vertex *p_vertexData,
                                              size_t p_VertexDataSize,
                                              const GLuint *p_IndexData,
                                              size_t p_IndexDataSize) {

  if (m_szVertexOccupiedSize + p_VertexDataSize >= m_szVertexBufferSize ||
      m_szIndexOccupiedSize + p_IndexDataSize >= m_szIndexBufferSize) {
    ResizeGLBuffer(p_VertexDataSize, p_IndexDataSize);
  }

  SAllocation l_allNewVAlloc;
  SAllocation l_allNewIAlloc;

  l_allNewVAlloc.alive = true;
  l_allNewVAlloc.generation = m_uiGlobalGeneration;
  l_allNewVAlloc.offset = m_szVertexCursor;
  l_allNewVAlloc.size = p_VertexDataSize;

  l_allNewIAlloc.alive = true;
  l_allNewIAlloc.generation = m_uiGlobalGeneration;
  l_allNewIAlloc.offset = m_szIndexCursor;
  l_allNewIAlloc.size = p_IndexDataSize;

  SBufferRange l_brVertexRange;
  SBufferRange l_brIndexRange;

  SBufferHandle l_bhVertexHandle;
  SBufferHandle l_bhIndexHandle;

  m_VertexAllocations.push_back(l_allNewVAlloc);
  m_IndexAllcoations.push_back(l_allNewIAlloc);

  uint32_t l_uiVertexAllocID = m_VertexAllocations.size() - 1;
  uint32_t l_uiIndexAllocID = m_IndexAllcoations.size() - 1;

  l_bhVertexHandle.allocationID = l_uiVertexAllocID;
  l_bhVertexHandle.bufferID = m_StaticStackID;
  l_bhVertexHandle.generation = m_uiGlobalGeneration;
  l_bhVertexHandle.slot = SlotType::VERTEX_SLOT;

  l_bhIndexHandle.allocationID = l_uiIndexAllocID;
  l_bhIndexHandle.bufferID = m_StaticStackID;
  l_bhIndexHandle.generation = m_uiGlobalGeneration;
  l_bhIndexHandle.slot = SlotType::INDEX_SLOT;

  l_brVertexRange.count = p_VertexDataSize / sizeof(Vertex);
  l_brVertexRange.dataType = TypeFlags::BUFFER_STATIC_MESH_DATA;
  l_brVertexRange.handle = l_bhVertexHandle;

  l_brIndexRange.count = p_IndexDataSize / sizeof(GLuint);
  l_brIndexRange.dataType = TypeFlags::BUFFER_STATIC_MESH_DATA;
  l_brIndexRange.handle = l_bhIndexHandle;

  glNamedBufferSubData(m_glVertexBuffer, l_allNewVAlloc.offset,
                       p_VertexDataSize, p_vertexData);

  glNamedBufferSubData(m_glIndexBuffer, l_allNewIAlloc.offset, p_IndexDataSize,
                       p_IndexData);

  m_szVertexCursor = l_allNewVAlloc.offset + l_allNewVAlloc.size;
  m_szIndexCursor = l_allNewIAlloc.offset + l_allNewIAlloc.size;

  m_szVertexOccupiedSize += p_VertexDataSize;
  m_szIndexOccupiedSize += p_IndexDataSize;

  return {l_brVertexRange, l_brIndexRange};
}

VertexIndexInfoPair CGLStaticStack::push_back(const MeshData &p_StaticData) {

  return push_back(p_StaticData.vertices.data(),
                   p_StaticData.vertices.size() * sizeof(Vertex),
                   p_StaticData.indecies.data(),
                   p_StaticData.indecies.size() * sizeof(GLuint));
}
void CGLStaticStack::InvalidateRange(const SBufferRange &p_range) {

  if (isRangeValid(p_range))
    switch (p_range.handle.slot) {
    case SlotType::VERTEX_SLOT: {

      m_VertexAllocations[p_range.handle.allocationID].alive = false;
      if (p_range.handle.allocationID == m_VertexAllocations.size() - 1) {
        pop_back();
      }
    } break;
    case SlotType::INDEX_SLOT: {

      m_IndexAllcoations[p_range.handle.allocationID].alive = false;
      if (p_range.handle.allocationID == m_IndexAllcoations.size() - 1) {
        pop_back();
      }
    } break;
    }
}
bool CGLStaticStack::isRangeValid(const SBufferRange &p_range) const {

  switch (p_range.handle.slot) {

  case SlotType::VERTEX_SLOT: {
    if (p_range.handle.allocationID >= m_VertexAllocations.size())
      return false;
    if (p_range.handle.generation != m_uiGlobalGeneration)
      return false;

    const SAllocation &l_allVertex =
        m_VertexAllocations[p_range.handle.allocationID];
    if (!l_allVertex.alive)
      return false;

    if (l_allVertex.offset + l_allVertex.size <= m_szVertexCursor)
      return true;
    else
      return false;

  } break;

  case SlotType::INDEX_SLOT: {
    if (p_range.handle.allocationID >= m_IndexAllcoations.size())
      return false;
    const SAllocation &l_allIndex =
        m_IndexAllcoations[p_range.handle.allocationID];
    if (p_range.handle.generation != m_uiGlobalGeneration)
      return false;
    if (l_allIndex.offset + l_allIndex.size <= m_szIndexCursor)
      return true;
    else
      return false;

  } break;

  case SlotType::SLOT_NONE: {
    SDL_Log("Error, no slot type added for allocation %d",
            p_range.handle.allocationID);
    return false;

  } break;
  default: {
    SDL_Log("Error, not supported slot type added for allocation %d",
            p_range.handle.allocationID);
    return false;

  } break;
  }
}
void CGLStaticStack::BindBuffer() { glBindVertexArray(m_glVertexArray); }
void CGLStaticStack::pop_back() {

  while (!m_VertexAllocations.empty()) {
    SAllocation &v = m_VertexAllocations.back();
    SAllocation &i = m_IndexAllcoations.back();

    // Stop if top is still alive
    if (v.alive || i.alive)
      break;

    // Rewind cursors
    m_szVertexCursor = v.offset;
    m_szIndexCursor = i.offset;

    m_szVertexOccupiedSize -= v.size;
    m_szIndexOccupiedSize -= i.size;

    m_VertexAllocations.pop_back();
    m_IndexAllcoations.pop_back();
  }
}

void CGLStaticStack::Destroy() {
  SDL_Log("\n\n\nSTATIC Stack DESTROYED \n\n\n");

  glDeleteBuffers(1, &m_glVertexBuffer);
  glDeleteBuffers(1, &m_glIndexBuffer);
  glDeleteVertexArrays(1, &m_glVertexArray);
}

void CGLStaticStack::Clear() {

  m_IndexAllcoations.clear();
  m_VertexAllocations.clear();

  m_szVertexCursor = 0;
  m_szIndexCursor = 0;

  m_szVertexOccupiedSize = 0;
  m_szIndexOccupiedSize = 0;

  m_uiGlobalGeneration++;
}
void CGLStaticStack::ResizeGLBuffer(size_t p_szMinimumSizeVertex = 0,
                                    size_t p_szMinimumSizeIndex = 0) {

  p_szMinimumSizeVertex =
      (p_szMinimumSizeVertex > 0) ? p_szMinimumSizeVertex : 1024UL;
  p_szMinimumSizeIndex =
      (p_szMinimumSizeIndex > 0) ? p_szMinimumSizeIndex : 1024UL;

  GLuint l_glNewVertexBuffer, l_glNewIndexBuffer, l_glNewVertexArray;

  size_t l_szOldVertBufferSize = m_szVertexBufferSize;
  size_t l_szOldIndexBufferSize = m_szIndexBufferSize;

  m_szVertexBufferSize =
      std::max(p_szMinimumSizeVertex, 2 * l_szOldVertBufferSize);
  m_szIndexBufferSize =
      std::max(p_szMinimumSizeIndex, 2 * l_szOldIndexBufferSize);

  glCreateBuffers(1, &l_glNewIndexBuffer);
  glCreateBuffers(1, &l_glNewVertexBuffer);
  glCreateVertexArrays(1, &l_glNewVertexArray);

  glNamedBufferData(l_glNewVertexBuffer, m_szVertexBufferSize, nullptr,
                    GL_DYNAMIC_DRAW);
  glNamedBufferData(l_glNewIndexBuffer, m_szIndexBufferSize, nullptr,
                    GL_DYNAMIC_DRAW);

  glCopyNamedBufferSubData(m_glVertexBuffer, l_glNewVertexBuffer, 0, 0,
                           m_szVertexOccupiedSize);

  glCopyNamedBufferSubData(m_glIndexBuffer, l_glNewIndexBuffer, 0, 0,
                           m_szIndexOccupiedSize);

  GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, UINT64_MAX);
  glDeleteSync(fence);

  Destroy();

  m_glVertexArray = l_glNewVertexArray;
  m_glVertexBuffer = l_glNewVertexBuffer;
  m_glIndexBuffer = l_glNewIndexBuffer;

  SetVertexAttribPointers();
}

void CGLStaticStack::SetVertexAttribPointers() {
  glBindVertexArray(m_glVertexArray);
  glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, Position));

  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, UV));

  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, Normal));

  glEnableVertexAttribArray(2);

  // Skeleton stuff

  glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex),
                         (void *)offsetof(Vertex, boneIDs));
  // glVertexAttribIPointer(3, 4, GL_INT, GL_FALSE, sizeof(Vertex), (void
  // *)offsetof(Vertex, boneIDs));
  glEnableVertexAttribArray(3);

  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, boneWeights));

  glEnableVertexAttribArray(4);
}

} // namespace eHazGraphics
