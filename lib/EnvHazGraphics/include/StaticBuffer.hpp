#ifndef ENVHAZGRAPHICS_STATICBUFFER_HPP
#define ENVHAZGRAPHICS_STATICBUFFER_HPP

#include "DataStructs.hpp"
#include <cstddef>
#include <vector>

namespace eHazGraphics {

class CStaticBuffer {
public:
  CStaticBuffer();

  CStaticBuffer(size_t p_szInitialVertSize, size_t p_szInitialIndexSize,
                uint32_t p_uiBufferID);

  VertexIndexInfoPair InsertIntoBuffer(const Vertex *p_VertexData,
                                       const size_t p_VertexDataSize,
                                       const GLuint *p_IndexData,
                                       const size_t p_IndexDataSize);

  uint32_t GetStaticBufferID() { return m_uintStaticBufferID; }

  void ClearBuffer();

  void BindBuffer();

  const SAllocation &GetAllocation(const SBufferRange &p_brRange);

  bool RemoveItem(const VertexIndexInfoPair &p_viipRange);

  void Destroy();

private:
  enum class StaticAllocType { Vertex, Index };

  void DefragmentAllocations(StaticAllocType p_type);

  uint32_t AllocateVertex(size_t p_szAllocSize);
  uint32_t AllocateIndex(size_t p_szAllocSize);

  uint32_t AllocateID(size_t p_szSize, StaticAllocType p_type);

  std::vector<SAllocation> m_VertexAllocations;
  std::vector<uint32_t> m_VertexAllocFreeIndecies;

  std::vector<SAllocation> m_IndexAllcoations;
  std::vector<uint32_t> m_IndexAllocFreeIndecies;

  GLuint m_glVertexBuffer;
  GLuint m_glIndexBuffer;

  GLuint m_glVertexArray;

  uint32_t m_uintStaticBufferID;

  void SetVertexAttribPointers();

  void ResizeBuffer();

protected:
  size_t GetOffsetToEnd(StaticAllocType p_type);

  // purely for quick access

  size_t m_szVertexBufferSize = 0;
  size_t m_szIndexBufferSize = 0;
};

} // namespace eHazGraphics

#endif
