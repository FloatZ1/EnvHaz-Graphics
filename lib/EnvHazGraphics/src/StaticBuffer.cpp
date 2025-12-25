#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "glad/glad.h"
#include <SDL3/SDL_log.h>
#include <StaticBuffer.hpp>
#include <algorithm>
#include <cstdint>
#include <unistd.h>

namespace eHazGraphics {

CStaticBuffer::CStaticBuffer() {}

void CStaticBuffer::DefragmentAllocations(StaticAllocType p_type) {
  auto &allocs = (p_type == StaticAllocType::Vertex) ? m_VertexAllocations
                                                     : m_IndexAllcoations;
  auto &freeIndices = (p_type == StaticAllocType::Vertex)
                          ? m_VertexAllocFreeIndecies
                          : m_IndexAllocFreeIndecies;

  // Sort allocations by offset for easier merging
  std::vector<std::pair<size_t, uint32_t>> offsetToIndex;
  for (uint32_t i = 0; i < allocs.size(); ++i) {
    if (!allocs[i].alive) {
      offsetToIndex.push_back({allocs[i].offset, i});
    }
  }
  std::sort(offsetToIndex.begin(), offsetToIndex.end());

  // Merge adjacent free blocks
  for (size_t i = 0; i < offsetToIndex.size(); ++i) {
    uint32_t currentIdx = offsetToIndex[i].second;
    SAllocation &current = allocs[currentIdx];

    // Look for adjacent free block
    for (size_t j = i + 1; j < offsetToIndex.size(); ++j) {
      uint32_t nextIdx = offsetToIndex[j].second;
      SAllocation &next = allocs[nextIdx];

      // If next block is immediately after current
      if (current.offset + current.size == next.offset) {
        // Merge next into current
        current.size += next.size;

        // Mark next as merged (size 0) - will be cleaned up
        next.size = 0;
        next.offset = 0;

        // Remove from free indices
        auto it = std::find(freeIndices.begin(), freeIndices.end(), nextIdx);
        if (it != freeIndices.end()) {
          freeIndices.erase(it);
        }
      }
    }
  }

  SDL_Log("Defragmentation complete for %s buffer",
          p_type == StaticAllocType::Vertex ? "vertex" : "index");
}

size_t CStaticBuffer::GetOffsetToEnd(StaticAllocType p_type) {

  const auto &allocs = (p_type == StaticAllocType::Vertex) ? m_VertexAllocations
                                                           : m_IndexAllcoations;

  size_t l_szEnd = 0;
  for (const auto &a : allocs) {
    if (a.alive) {
      l_szEnd = std::max(l_szEnd, a.offset + a.size);
    }
  }
  return l_szEnd;
}
uint32_t CStaticBuffer::AllocateVertex(size_t p_szAllocSize) {

  uint32_t l_uiReturnID = -1;

  if (p_szAllocSize >= m_szVertexBufferSize) {

    SDL_Log("ERROR: SIZE DOES NOT FIT: %zu", p_szAllocSize);

    return -1;
  }

  for (int l_attempt = 0; l_attempt < 2; ++l_attempt) {

    if (l_attempt == 1) {
      SDL_Log("First allocation attempt failed, defragmenting...");
      DefragmentAllocations(StaticAllocType::Vertex);
    }

    for (uint32_t i = 0; i < m_VertexAllocFreeIndecies.size(); ++i) {
      uint32_t ID = m_VertexAllocFreeIndecies[i];

      if (m_VertexAllocations[ID].size > p_szAllocSize) {

        SAllocation l_allNewAlloc;
        l_allNewAlloc.size = m_VertexAllocations[ID].size - p_szAllocSize;
        l_allNewAlloc.alive = false;
        l_allNewAlloc.offset = m_VertexAllocations[ID].offset + p_szAllocSize;
        l_allNewAlloc.generation++;

        l_uiReturnID = ID;

        m_VertexAllocations[ID].alive = true;
        m_VertexAllocations[ID].generation++;
        m_VertexAllocations[ID].size = p_szAllocSize;
        if (m_VertexAllocFreeIndecies.size() > 1) {
          std::iter_swap(m_VertexAllocFreeIndecies.begin() + i,
                         m_VertexAllocFreeIndecies.end() - 1);
          m_VertexAllocFreeIndecies.pop_back();
        } else {
          m_VertexAllocFreeIndecies.clear();
        }

        m_VertexAllocations.push_back(l_allNewAlloc);
        m_VertexAllocFreeIndecies.push_back(
            static_cast<uint32_t>(m_VertexAllocations.size() - 1));
        break;

      } else if (m_VertexAllocations[ID].size == p_szAllocSize) {

        l_uiReturnID = ID;
        m_VertexAllocations[ID].alive = true;
        m_VertexAllocations[ID].generation++;
        m_VertexAllocations[ID].size = p_szAllocSize;
        if (m_VertexAllocFreeIndecies.size() > 1) {
          std::iter_swap(m_VertexAllocFreeIndecies.begin() + i,
                         m_VertexAllocFreeIndecies.end() - 1);
          m_VertexAllocFreeIndecies.pop_back();
        } else {
          m_VertexAllocFreeIndecies.clear();
        }
        break;
        // --i;
      }
    }
  }

  return l_uiReturnID;
}
uint32_t CStaticBuffer::AllocateIndex(size_t p_szAllocSize) {

  uint32_t l_uiReturnID = -1;

  if (p_szAllocSize >= m_szIndexBufferSize) {

    SDL_Log("ERROR: SIZE DOES NOT FIT: %zu", p_szAllocSize);

    return -1;
  }
  for (int l_attempt = 0; l_attempt < 2; ++l_attempt) {

    if (l_attempt == 1) {
      SDL_Log("First allocation attempt failed, defragmenting...");
      DefragmentAllocations(StaticAllocType::Index);
    }

    for (uint32_t i = 0; i < m_IndexAllocFreeIndecies.size(); ++i) {
      uint32_t ID = m_IndexAllocFreeIndecies[i];

      if (m_IndexAllcoations[ID].size > p_szAllocSize) {

        SAllocation l_allNewAlloc;
        l_allNewAlloc.size = m_IndexAllcoations[ID].size - p_szAllocSize;
        l_allNewAlloc.alive = false;
        l_allNewAlloc.offset = m_IndexAllcoations[ID].offset + p_szAllocSize;
        l_allNewAlloc.generation++;

        l_uiReturnID = ID;

        m_IndexAllcoations[ID].alive = true;
        m_IndexAllcoations[ID].generation++;
        m_IndexAllcoations[ID].size = p_szAllocSize;
        if (m_IndexAllocFreeIndecies.size() > 1) {
          std::iter_swap(m_IndexAllocFreeIndecies.begin() + i,
                         m_IndexAllocFreeIndecies.end() - 1);
          m_IndexAllocFreeIndecies.pop_back();
        } else {
          m_IndexAllocFreeIndecies.clear();
        }

        m_IndexAllcoations.push_back(l_allNewAlloc);
        m_IndexAllocFreeIndecies.push_back(
            static_cast<uint32_t>(m_IndexAllcoations.size() - 1));
        break;

      } else if (m_IndexAllcoations[ID].size == p_szAllocSize) {

        l_uiReturnID = ID;
        m_IndexAllcoations[ID].alive = true;
        m_IndexAllcoations[ID].generation++;
        m_IndexAllcoations[ID].size = p_szAllocSize;
        if (m_IndexAllocFreeIndecies.size() > 1) {
          std::iter_swap(m_IndexAllocFreeIndecies.begin() + i,
                         m_IndexAllocFreeIndecies.end() - 1);
          m_IndexAllocFreeIndecies.pop_back();
        } else {
          m_IndexAllocFreeIndecies.clear();
        }
        break;
        // --i;
      }
    }
  }
  return l_uiReturnID;
}

CStaticBuffer::CStaticBuffer(size_t p_szInitialVertSize,
                             size_t p_szInitialIndexSize, uint32_t p_uiBufferID)
    : m_uintStaticBufferID(p_uiBufferID) {
  m_szIndexBufferSize = p_szInitialIndexSize;
  m_szVertexBufferSize = p_szInitialVertSize;
  glCreateBuffers(1, &m_glVertexBuffer);
  glCreateVertexArrays(1, &m_glVertexArray);
  glCreateBuffers(1, &m_glIndexBuffer);

  glNamedBufferData(m_glVertexBuffer, p_szInitialVertSize, nullptr,
                    GL_STATIC_DRAW);
  glNamedBufferData(m_glIndexBuffer, p_szInitialIndexSize, nullptr,
                    GL_STATIC_DRAW);

  SAllocation l_allVertex;
  l_allVertex.alive = false;
  l_allVertex.generation = 0;
  l_allVertex.offset = 0;
  l_allVertex.size = m_szVertexBufferSize;

  SAllocation l_allIndex;
  l_allIndex.alive = false;
  l_allIndex.offset = 0;
  l_allIndex.size = m_szIndexBufferSize;
  l_allIndex.generation = 0;

  m_IndexAllcoations.push_back(l_allIndex);
  m_VertexAllocations.push_back(l_allVertex);

  m_VertexAllocFreeIndecies.push_back(0);
  m_IndexAllocFreeIndecies.push_back(0);

  SetVertexAttribPointers();
}

void CStaticBuffer::ClearBuffer() {

  m_IndexAllcoations.clear();
  m_VertexAllocations.clear();

  m_VertexAllocFreeIndecies.clear();
  m_IndexAllocFreeIndecies.clear();

  SAllocation l_allVertex;
  l_allVertex.alive = false;
  l_allVertex.generation = 0;
  l_allVertex.offset = 0;
  l_allVertex.size = m_szVertexBufferSize;

  SAllocation l_allIndex;
  l_allIndex.alive = false;
  l_allIndex.offset = 0;
  l_allIndex.size = m_szIndexBufferSize;
  l_allIndex.generation = 0;

  m_IndexAllcoations.push_back(l_allIndex);
  m_VertexAllocations.push_back(l_allVertex);

  m_VertexAllocFreeIndecies.push_back(0);
  m_IndexAllocFreeIndecies.push_back(0);
}

void CStaticBuffer::BindBuffer() { glBindVertexArray(m_glVertexArray); }

bool CStaticBuffer::RemoveItem(const VertexIndexInfoPair &p_viipRange) {

  // NOTE:make better later

  uint32_t l_uiVallocID = p_viipRange.first.handle.allocationID;
  uint32_t l_uiIallocID = p_viipRange.second.handle.allocationID;
  if (l_uiIallocID < m_IndexAllcoations.size() &&
      l_uiVallocID < m_VertexAllocations.size() && l_uiIallocID >= 0 &&
      l_uiVallocID >= 0) {

    const SBufferRange &l_brVertex = p_viipRange.first;
    const SBufferRange &l_brIndex = p_viipRange.second;

    uint32_t l_uiVertAllocID = l_brVertex.handle.allocationID;
    uint32_t l_uiIndexAllocID = l_brIndex.handle.allocationID;
    m_IndexAllcoations[l_uiIndexAllocID].alive = false;
    m_VertexAllocations[l_uiVertAllocID].alive = false;

    m_IndexAllocFreeIndecies.push_back(l_uiIndexAllocID);
    m_VertexAllocFreeIndecies.push_back(l_uiVertAllocID);
    return true;
  } else {
    SDL_Log("ERROR: INVALID STATIC BUFFER REMOVAL: ALLOCATION ID IS INVALID");
    return false;
  }
}

void CStaticBuffer::Destroy() {
  SDL_Log("\n\n\nSTATIC BUFFER DESTROYED \n\n\n");
  glDeleteBuffers(1, &m_glVertexBuffer);
  glDeleteBuffers(1, &m_glIndexBuffer);
}

uint32_t CStaticBuffer::AllocateID(size_t p_szSize, StaticAllocType p_type) {

  switch (p_type) {

  case StaticAllocType::Vertex: {
    return AllocateVertex(p_szSize);
  } break;
  case StaticAllocType::Index: {
    return AllocateIndex(p_szSize);

  } break;
  }
}

void CStaticBuffer::SetVertexAttribPointers() {

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

void CStaticBuffer::ResizeBuffer() {

  GLuint l_glNewIndexBuffer, l_glNewVertexBuffer;

  glCreateBuffers(1, &l_glNewIndexBuffer);
  glCreateBuffers(1, &l_glNewVertexBuffer);

  size_t l_szOldVertBufferSize = m_szVertexBufferSize;
  size_t l_szOldIndexBufferSize = m_szIndexBufferSize;

  m_szVertexBufferSize = std::max(1024UL, 2 * l_szOldVertBufferSize);
  m_szIndexBufferSize = std::max(1024UL, 2 * l_szOldIndexBufferSize);

  glNamedBufferData(l_glNewVertexBuffer, m_szVertexBufferSize, nullptr,
                    GL_STATIC_DRAW);
  glNamedBufferData(l_glNewIndexBuffer, m_szIndexBufferSize, nullptr,
                    GL_STATIC_DRAW);

  glCopyNamedBufferSubData(m_glVertexBuffer, l_glNewVertexBuffer, 0, 0,
                           l_szOldVertBufferSize);

  glCopyNamedBufferSubData(m_glIndexBuffer, l_glNewIndexBuffer, 0, 0,
                           l_szOldIndexBufferSize);

  Destroy();

  m_glIndexBuffer = l_glNewIndexBuffer;
  m_glVertexBuffer = l_glNewVertexBuffer;

  SetVertexAttribPointers();

  uint32_t l_uiVtail = AllocateID(
      (m_szVertexBufferSize - l_szOldVertBufferSize), StaticAllocType::Vertex);

  uint32_t l_uiItail = AllocateID(
      (m_szIndexBufferSize - l_szOldIndexBufferSize), StaticAllocType::Index);

  m_IndexAllcoations[l_uiItail].alive = false;
  m_VertexAllocations[l_uiVtail].alive = false;

  m_IndexAllocFreeIndecies.push_back(l_uiItail);
  m_VertexAllocFreeIndecies.push_back(l_uiVtail);
}

VertexIndexInfoPair CStaticBuffer::InsertIntoBuffer(
    const Vertex *p_VertexData, const size_t p_VertexDataSize,
    const GLuint *p_IndexData, const size_t p_IndexDataSize) {

  uint32_t l_uiVallocID = AllocateID(p_VertexDataSize, StaticAllocType::Vertex);

  uint32_t l_uiIallocID = AllocateID(p_IndexDataSize, StaticAllocType::Index);

  while (l_uiVallocID == -1) {

    ResizeBuffer();

    l_uiVallocID = AllocateID(p_VertexDataSize, StaticAllocType::Vertex);
  }

  while (l_uiIallocID == -1) {

    ResizeBuffer();

    l_uiIallocID = AllocateID(p_IndexDataSize, StaticAllocType::Index);
  }

  const SAllocation &l_alVertexAlloc = m_VertexAllocations[l_uiVallocID];

  const SAllocation &l_alIndexAlloc = m_IndexAllcoations[l_uiIallocID];

  glNamedBufferSubData(m_glVertexBuffer, l_alVertexAlloc.offset,
                       p_VertexDataSize, p_VertexData);

  glNamedBufferSubData(m_glIndexBuffer, l_alIndexAlloc.offset, p_IndexDataSize,
                       p_IndexData);

  SBufferHandle l_hVertexHandle{m_uintStaticBufferID, SlotType::VERTEX_SLOT,
                                l_uiVallocID, l_alVertexAlloc.generation};
  SBufferHandle l_hIndexHandle{m_uintStaticBufferID, SlotType::INDEX_SLOT,
                               l_uiIallocID, l_alIndexAlloc.generation};

  SBufferRange l_brVertexRange;
  SBufferRange l_brIndexRange;

  l_brIndexRange.count = static_cast<int>(p_IndexDataSize / sizeof(GLuint));
  l_brVertexRange.count = static_cast<int>(p_VertexDataSize / sizeof(Vertex));

  l_brVertexRange.dataType = TypeFlags::BUFFER_STATIC_MESH_DATA;
  l_brIndexRange.dataType = TypeFlags::BUFFER_STATIC_MESH_DATA;

  l_brVertexRange.handle = l_hVertexHandle;
  l_brIndexRange.handle = l_hIndexHandle;

  return {l_brVertexRange, l_brIndexRange};
}

const SAllocation &CStaticBuffer::GetAllocation(const SBufferRange &p_brRange) {

  switch (p_brRange.handle.slot) {

  case SlotType::VERTEX_SLOT: {

    return m_VertexAllocations[p_brRange.handle.allocationID];

    break;
  }
  case SlotType::INDEX_SLOT: {
    return m_IndexAllcoations[p_brRange.handle.allocationID];
    break;
  }
  }
}
} // namespace eHazGraphics
