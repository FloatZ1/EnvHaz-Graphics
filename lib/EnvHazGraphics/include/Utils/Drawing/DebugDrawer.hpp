#ifndef ENVHAZGRAPHICS_DEBUG_DRAWING_HPP
#define ENVHAZGRAPHICS_DEBUG_DRAWING_HPP

#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include "ShaderManager.hpp"
#include "StaticStack.hpp"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include <vector>
namespace eHazGraphics {

class DebugDrawer {

  enum MeshType { Cube, Sphere, Line };

  struct DebugDrawCommand {
    uint32_t vertexOffset;   // in elements
    size_t indexOffset;      // in elements, 0 for non-indexed meshes
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

  void SubmitLine(const glm::vec3 &start, const glm::vec3 &end, float thickness,
                  const glm::vec4 &color);

  void SubmitCube(glm::vec3 position, glm::vec3 halfExtents, glm::vec4 color);

  void SubmitSphere(glm::vec3 position, float size);

  void DrawDebug(glm::vec3 cameraPos);

private:
  std::vector<DebugMesh> debugMeshes;

  std::vector<DebugInstance> lineInstances;
  std::vector<DebugInstance> cubeInstances;
  std::vector<DebugInstance> sphereInstances;

  ShaderComboID lineShader;
  ShaderComboID meshShader;

  ShaderManager *shaderManager;
  BufferManager *bufferManager;
  SBufferRange instanceBR;
  // DebugDrawCommand lineDrawCommand;
  // DebugDrawCommand cubeDrawCommand;
  // DebugDrawCommand sphereDrawCommand;

  DrawElementsIndirectCommand lineCommand;
  DrawElementsIndirectCommand cubeCommand;
  DrawElementsIndirectCommand sphereCommand;

  CGLStaticStack staticStack;

  // 4 vertices: x = 0/1 along the line, y/z = -0.5 to 0.5 for width/thickness
  static constexpr glm::vec3 lineQuadVertices[] = {
      {0.0f, -0.5f, -0.5f}, // vertex 0: start, bottom-left
      {0.0f, 0.5f, -0.5f},  // vertex 1: start, top-left
      {1.0f, -0.5f, -0.5f}, // vertex 2: end, bottom-right
      {1.0f, 0.5f, -0.5f}   // vertex 3: end, top-right
  };

  static constexpr GLuint lineQuadIndices[] = {
      0, 1, 2, // first triangle
      2, 1, 3  // second triangle
  };

  // 8 vertices

  static constexpr glm::vec3 cubeVertices[] = {
      {1.0f, 1.0f, -1.0f},   // 0
      {1.0f, -1.0f, -1.0f},  // 1
      {1.0f, 1.0f, 1.0f},    // 2
      {1.0f, -1.0f, 1.0f},   // 3
      {-1.0f, 1.0f, -1.0f},  // 4
      {-1.0f, -1.0f, -1.0f}, // 5
      {-1.0f, 1.0f, 1.0f},   // 6
      {-1.0f, -1.0f, 1.0f}   // 7
  };

  // Index buffer (12 triangles, 36 indices)

  static constexpr uint32_t cubeIndices[] = {
      // +X face
      0, 4, 6, 0, 6, 2,

      // +Z face
      3, 2, 6, 3, 6, 7,

      // -X face
      7, 6, 4, 7, 4, 5,

      // -Z face
      5, 1, 3, 5, 3, 7,

      // +Y face
      1, 0, 2, 1, 2, 3,

      // -Y face
      5, 4, 0, 5, 0, 1};

  // Indices for triangles (coarse, 16 triangles example)

  static constexpr glm::vec3 sphereVertices[] = {
      {0.000000f, -1.000000f, 0.000000f},   // 0
      {0.723607f, -0.447220f, 0.525725f},   // 1
      {-0.276388f, -0.447220f, 0.850649f},  // 2
      {-0.894426f, -0.447216f, 0.000000f},  // 3
      {-0.276388f, -0.447220f, -0.850649f}, // 4
      {0.723607f, -0.447220f, -0.525725f},  // 5
      {0.276388f, 0.447220f, 0.850649f},    // 6
      {-0.723607f, 0.447220f, 0.525725f},   // 7
      {-0.723607f, 0.447220f, -0.525725f},  // 8
      {0.276388f, 0.447220f, -0.850649f},   // 9
      {0.894426f, 0.447216f, 0.000000f},    // 10
      {0.000000f, 1.000000f, 0.000000f},    // 11
      {-0.162456f, -0.850654f, 0.499995f},  // 12
      {0.425323f, -0.850654f, 0.309011f},   // 13
      {0.262869f, -0.525738f, 0.809012f},   // 14
      {0.850648f, -0.525736f, 0.000000f},   // 15
      {0.425323f, -0.850654f, -0.309011f},  // 16
      {-0.525730f, -0.850652f, 0.000000f},  // 17
      {-0.688189f, -0.525736f, 0.499997f},  // 18
      {-0.162456f, -0.850654f, -0.499995f}, // 19
      {-0.688189f, -0.525736f, -0.499997f}, // 20
      {0.262869f, -0.525738f, -0.809012f},  // 21
      {0.951058f, 0.000000f, 0.309013f},    // 22
      {0.951058f, 0.000000f, -0.309013f},   // 23
      {0.000000f, 0.000000f, 1.000000f},    // 24
      {0.587786f, 0.000000f, 0.809017f},    // 25
      {-0.951058f, 0.000000f, 0.309013f},   // 26
      {-0.587786f, 0.000000f, 0.809017f},   // 27
      {-0.587786f, 0.000000f, -0.809017f},  // 28
      {-0.951058f, 0.000000f, -0.309013f},  // 29
      {0.587786f, 0.000000f, -0.809017f},   // 30
      {0.000000f, 0.000000f, -1.000000f},   // 31
      {0.688189f, 0.525736f, 0.499997f},    // 32
      {-0.262869f, 0.525738f, 0.809012f},   // 33
      {-0.850648f, 0.525736f, 0.000000f},   // 34
      {-0.262869f, 0.525738f, -0.809012f},  // 35
      {0.688189f, 0.525736f, -0.499997f},   // 36
      {0.162456f, 0.850654f, 0.499995f},    // 37
      {0.525730f, 0.850652f, 0.000000f},    // 38
      {-0.425323f, 0.850654f, 0.309011f},   // 39
      {-0.425323f, 0.850654f, -0.309011f},  // 40
      {0.162456f, 0.850654f, -0.499995f}    // 41
  };

  static constexpr GLuint sphereIndices[] = {
      0,  13, 12, 1,  13, 15, 0,  12, 17, 0,  17, 19, 0,  19, 16, 1,  15,
      22, 2,  14, 24, 3,  18, 26, 4,  20, 28, 5,  21, 30, 1,  22, 25, 2,
      24, 27, 3,  26, 29, 4,  28, 31, 5,  30, 23, 6,  32, 37, 7,  33, 39,
      8,  34, 40, 9,  35, 41, 10, 36, 38, 38, 41, 11, 38, 36, 41, 36, 9,
      41, 41, 40, 11, 41, 35, 40, 35, 8,  40, 40, 37, 11, 40, 33, 37, 33,
      6,  37, 37, 38, 11, 37, 32, 38, 32, 10, 38, 23, 36, 10, 23, 30, 36,
      30, 9,  36, 31, 35, 9,  31, 28, 35, 28, 8,  35, 29, 33, 8,  29, 26,
      33, 26, 7,  33, 25, 32, 6,  25, 22, 32, 22, 1,  32, 22, 23, 10, 22,
      15, 23, 15, 5,  23, 16, 21, 5,  16, 19, 21, 19, 4,  21, 19, 20, 4,
      19, 17, 20, 17, 3,  20, 17, 18, 3,  17, 12, 18, 12, 2,  18, 15, 16,
      5,  15, 13, 16, 13, 0,  16, 12, 14, 2,  12, 13, 14, 13, 1,  14};
};
} // namespace eHazGraphics

#endif
