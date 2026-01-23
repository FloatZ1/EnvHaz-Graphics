#ifndef ENVHAZGRAPHICS_DEBUG_DRAWING_HPP
#define ENVHAZGRAPHICS_DEBUG_DRAWING_HPP

#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include "ShaderManager.hpp"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include <vector>
namespace eHazGraphics {

class DebugDrawer {

  enum MeshType { Cube, Sphere, Line };

  struct DebugDrawCommand {
    uint32_t vertexOffset;   // in elements
    uint32_t indexOffset;    // in elements, 0 for non-indexed meshes
    uint32_t indexCount;     // 0 for non-indexed
    uint32_t instanceOffset; // offset into the instance SSBO
    uint32_t instanceCount;  // number of instances
  };

  struct DebugInstance {

    glm::mat4 model; // 64 bytes
    glm::vec4 color; // optional, 16 bytes
  };
  struct DebugMesh {

    SBufferRange vertexRange;
    std::optional<SBufferRange>
        indexRange; // invalid / empty for non-indexed meshes
  };

public:
  DebugDrawer(ShaderManager *shaderManager, BufferManager *bufferManager);

  void SubmitLine(glm::vec3 start, glm::vec3 end, float width, glm::vec4 color);

  void SubmitCube(glm::vec3 position, glm::vec3 halfExtents, glm::vec3 color);

  void SubmitSphere(glm::vec3 position, float size);

  void DrawDebug();

private:
  std::vector<DebugMesh> debugMeshes;

  GLuint m_glDebugVAO;

  std::vector<DebugInstance> lineInstances;
  std::vector<DebugInstance> cubeInstances;
  std::vector<DebugInstance> sphereInstances;

  ShaderComboID lineShader;
  ShaderComboID meshShader;

  ShaderManager *shaderManager;
  BufferManager *bufferManager;
  SBufferRange instanceBR;
  DebugDrawCommand lineDrawCommand;
  DebugDrawCommand cubeDrawCommand;
  DebugDrawCommand sphereDrawCommand;

  // 4 vertices: x = 0/1 along the line, y/z = -0.5 to 0.5 for width/thickness
  static constexpr float lineQuadVertices[] = {
      0.0f, -0.5f, -0.5f, // vertex 0: start, bottom-left
      0.0f, 0.5f,  -0.5f, // vertex 1: start, top-left
      1.0f, -0.5f, -0.5f, // vertex 2: end, bottom-right
      1.0f, 0.5f,  -0.5f  // vertex 3: end, top-right
  };

  static constexpr unsigned int lineQuadIndices[] = {
      0, 1, 2, // first triangle
      2, 1, 3  // second triangle
  };

  // 8 vertices
  static constexpr float cubeVertices[] = {
      -0.5f, -0.5f, -0.5f, // 0
      0.5f,  -0.5f, -0.5f, // 1
      0.5f,  0.5f,  -0.5f, // 2
      -0.5f, 0.5f,  -0.5f, // 3
      -0.5f, -0.5f, 0.5f,  // 4
      0.5f,  -0.5f, 0.5f,  // 5
      0.5f,  0.5f,  0.5f,  // 6
      -0.5f, 0.5f,  0.5f   // 7
  };

  // Index buffer (12 triangles, 36 indices)
  static constexpr unsigned int cubeIndices[] = {
      0, 1, 2, 2, 3, 0, // back
      4, 5, 6, 6, 7, 4, // front
      4, 7, 3, 3, 0, 4, // left
      1, 5, 6, 6, 2, 1, // right
      4, 5, 1, 1, 0, 4, // bottom
      3, 2, 6, 6, 7, 3  // top
  };

  static constexpr float sphereVertices[] = {
      0.0f,  0.5f,   0.0f,  0.47f, 0.15f,  0.0f,  0.29f, 0.15f,
      0.4f,  -0.29f, 0.15f, 0.4f,  -0.47f, 0.15f, 0.0f,  -0.29f,
      0.15f, -0.4f,  0.29f, 0.15f, -0.4f,  0.0f,  -0.5f, 0.0f};

  // Indices for triangles (coarse, 16 triangles example)
  static constexpr unsigned int sphereIndices[] = {
      0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 1, // top cap
      7, 2, 1, 7, 3, 2, 7, 4, 3, 7, 5, 4, 7, 6, 5, 7, 1, 6  // bottom cap
  };
};

} // namespace eHazGraphics

#endif
