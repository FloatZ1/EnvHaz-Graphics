#ifndef EHAZ_GRAPHICS_STATIC_STACK_HPP
#define EHAZ_GRAPHICS_STATIC_STACK_HPP

#include "DataStructs.hpp"
#include "glad/glad.h"
#include <optional>
#include <unistd.h>
#include <vector>
namespace eHazGraphics {

class CGLStaticStack {
public:
  CGLStaticStack();

  CGLStaticStack(size_t p_szInitialVertSize, size_t p_szInitialIndexSize,
                 uint32_t p_StaticStackID);

  VertexIndexInfoPair push_back(const Vertex *p_vertexData,
                                size_t p_VertexDataSize,
                                const GLuint *p_IndexData,
                                size_t p_IndexDataSize);

  VertexIndexInfoPair push_back(const MeshData &p_StaticData);

  void Clear();

  bool isRangeValid(const SBufferRange &p_range) const;

  inline bool isRangeValid(const VertexIndexInfoPair &p_pair) {
    return (isRangeValid(p_pair.first) && isRangeValid(p_pair.second)) ? true
                                                                       : false;
  }

  void InvalidateRange(const SBufferRange &p_range);

  inline void InvalidateRange(const VertexIndexInfoPair &p_pair) {
    InvalidateRange(p_pair.first);
    InvalidateRange(p_pair.second);
  }

  std::optional<SAllocation> GetAllocation(const SBufferRange &p_range) const;

  void BindBuffer();

  uint32_t GetStaticStackID() const { return m_StaticStackID; }

  void pop_back();

  void Destroy();

private:
  void ResizeGLBuffer(size_t p_szMinimumSizeVertex,
                      size_t p_szMinimumSizeIndex);

  void SetVertexAttribPointers();

  std::vector<SAllocation> m_VertexAllocations;
  std::vector<SAllocation> m_IndexAllcoations;

  uint32_t m_uiGlobalGeneration = 0;

  GLuint m_glVertexArray = 0;
  GLuint m_glVertexBuffer = 0;
  GLuint m_glIndexBuffer = 0;
  uint32_t m_StaticStackID = 0;

  size_t m_szVertexBufferSize = 0; // full size of current buffer
  size_t m_szIndexBufferSize = 0;

  size_t m_szVertexOccupiedSize = 0;
  size_t m_szIndexOccupiedSize = 0;

  size_t m_szVertexCursor = 0; // from where to start writting
  size_t m_szIndexCursor = 0;
};

} // namespace eHazGraphics

#endif
