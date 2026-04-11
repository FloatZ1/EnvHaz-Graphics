#ifndef EnvHazGraphics_SINGLE_DRAW_BUFFER_HPP
#define EnvHazGraphics_SINGLE_DRAW_BUFFER_HPP

#include "DataStructs.hpp"
#include "glad/glad.h"
#include <SDL3/SDL_log.h>
#include <cstdint>

using namespace eHazGraphics;

namespace eHazGraphics_Utils {

class CSingleDrawBuffer {

public:
  void Initialize() {

    glCreateBuffers(1, &m_uiVBO);
    glCreateBuffers(1, &m_uiEBO);

    glCreateVertexArrays(1, &m_uiVAO);

    SetVertexAttribPointers();
  }

  void SetBufferData(const eHazGraphics::MeshData &data,
                     glm::mat4 relativeMatrix) {

    glNamedBufferData(m_uiVBO, sizeof(Vertex) * data.vertices.size(),
                      data.vertices.data(), GL_STATIC_DRAW);
    glNamedBufferData(m_uiEBO, sizeof(GLuint) * data.indecies.size(),
                      data.indecies.data(), GL_STATIC_DRAW);

    m_uiIndexCount = data.indecies.size();
    m_uiVertexCount = data.vertices.size();

    m_mat4LocalMatrix = relativeMatrix;
    if (data.indecies.size() == 0 || data.vertices.size() == 0)
      SDL_Log("ERROR: NO MESH DATA PROVIDED FOR SINGLE DRAW BUFFER\n");
  }

  glm::mat4 GetRelativeMatrix() { return m_mat4LocalMatrix; }

  // Make sure to bind a shader before calling this
  void Draw() {
    GLint previousVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVAO);

    glBindVertexArray(m_uiVAO);
    glDrawElements(GL_TRIANGLES, m_uiIndexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(previousVAO);
  }

  void Destroy() {

    glDeleteBuffers(1, &m_uiVBO);
    glDeleteBuffers(1, &m_uiEBO);
    glDeleteVertexArrays(1, &m_uiVAO);
  }

private:
  GLuint m_uiVAO, m_uiVBO, m_uiEBO;

  uint32_t m_uiIndexCount = 0;
  uint32_t m_uiVertexCount = 0;

  glm::mat4 m_mat4LocalMatrix;

  void SetVertexAttribPointers() {
    glBindVertexArray(m_uiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_uiVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiEBO);

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

    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Tangent));
    glEnableVertexAttribArray(5);

    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Bitangent));
    glEnableVertexAttribArray(6);

    glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, UV2));

    glEnableVertexAttribArray(7);
  }
};

} // namespace eHazGraphics_Utils

#endif
