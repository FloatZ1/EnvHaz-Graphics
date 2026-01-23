

#include "Utils/Drawing/DebugDrawer.hpp"
#include "DataStructs.hpp"
#include "glad/glad.h"
#include "glm/ext/matrix_transform.hpp"
#include <optional>
#include <vector>

namespace eHazGraphics {

DebugDrawer::DebugDrawer(ShaderManager *shaderManager,
                         BufferManager *bufferManager)
    : shaderManager(shaderManager), bufferManager(bufferManager)

{

  lineShader = shaderManager->CreateShaderProgramme(
      RESOURCES_PATH "debug_lines.vert", RESOURCES_PATH "debug_shapes.frag");

  meshShader = shaderManager->CreateShaderProgramme(
      RESOURCES_PATH "debug_shapes.vert", RESOURCES_PATH "debug_shapes.frag");

  glCreateVertexArrays(1, &m_glDebugVAO);

  glBindVertexArray(m_glDebugVAO);
  glBindBuffer(
      GL_ELEMENT_ARRAY_BUFFER,
      bufferManager->GetGLBufferID(TypeFlags::BUFFER_DEBUG_SHAPE_DATA)[0]);

  SBufferRange lineVertexRange = bufferManager->InsertNewDynamicData(
      lineQuadVertices, sizeof(lineQuadVertices),
      TypeFlags::BUFFER_DEBUG_SHAPE_DATA);

  SBufferRange lineIndexRange = bufferManager->InsertNewDynamicData(
      lineQuadIndices, sizeof(lineQuadIndices),
      TypeFlags::BUFFER_DEBUG_SHAPE_DATA);

  SBufferRange cubeVertexRange = bufferManager->InsertNewDynamicData(
      cubeVertices, sizeof(cubeVertices), TypeFlags::BUFFER_DEBUG_SHAPE_DATA);

  SBufferRange cubeIndexRange = bufferManager->InsertNewDynamicData(
      cubeIndices, sizeof(cubeIndices), TypeFlags::BUFFER_DEBUG_SHAPE_DATA);

  SBufferRange sphereVertexRange = bufferManager->InsertNewDynamicData(
      sphereVertices, sizeof(sphereVertices),
      TypeFlags::BUFFER_DEBUG_SHAPE_DATA);

  SBufferRange sphereIndexRange = bufferManager->InsertNewDynamicData(
      sphereIndices, sizeof(sphereIndices), TypeFlags::BUFFER_DEBUG_SHAPE_DATA);

  DebugMesh lineMesh{lineVertexRange, lineIndexRange};

  DebugMesh cubeMesh{cubeVertexRange, cubeIndexRange};

  DebugMesh sphereMesh{sphereVertexRange, sphereIndexRange};

  auto lineVertexAlloc = bufferManager->GetAllocation(lineVertexRange);

  auto lineIndexAlloc = bufferManager->GetAllocation(lineIndexRange);

  auto cubeVertexAlloc = bufferManager->GetAllocation(cubeVertexRange);

  auto cubeIndexAlloc = bufferManager->GetAllocation(cubeIndexRange);

  auto sphereVertexAlloc = bufferManager->GetAllocation(sphereVertexRange);

  auto sphereIndexAlloc = bufferManager->GetAllocation(sphereIndexRange);

  lineDrawCommand.indexCount = sizeof(lineQuadIndices) / sizeof(GLuint);

  lineDrawCommand.indexOffset = lineIndexAlloc.value().offset / sizeof(GLuint);
  lineDrawCommand.vertexOffset = lineVertexAlloc.value().offset / sizeof(float);

  cubeDrawCommand.indexCount = cubeIndexAlloc.value().size / sizeof(GLuint);
  cubeDrawCommand.indexOffset = cubeIndexAlloc.value().offset / sizeof(GLuint);
  cubeDrawCommand.vertexOffset = cubeVertexAlloc.value().offset / sizeof(float);

  sphereDrawCommand.indexCount = sphereIndexAlloc.value().size / sizeof(GLuint);
  sphereDrawCommand.indexOffset =
      sphereIndexAlloc.value().offset / sizeof(GLuint);
  sphereDrawCommand.vertexOffset =
      sphereVertexAlloc.value().offset / sizeof(float);

  debugMeshes = {lineMesh, cubeMesh, sphereMesh};
}

void DebugDrawer::SubmitLine(glm::vec3 start, glm::vec3 end, float width,
                             glm::vec4 color) {
  glm::vec3 dir = end - start;
  float length = glm::length(dir);
  if (length < 0.0001f)
    return; // skip degenerate lines

  // Model matrix: translate to start, scale along line and width
  glm::mat4 model = glm::translate(glm::mat4(1.0f), start) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(length, width, 1.0f));

  lineInstances.push_back({model, color});
}

void DebugDrawer::SubmitCube(glm::vec3 position, glm::vec3 halfExtents,
                             glm::vec3 color) {
  // Model matrix: translate to position, scale by half-extents
  glm::mat4 model = glm::translate(glm::mat4(1.0f), position) *
                    glm::scale(glm::mat4(1.0f), halfExtents);

  cubeInstances.push_back({model, glm::vec4(color, 1.0f)});
}

void DebugDrawer::SubmitSphere(glm::vec3 position, float size) {
  // Model matrix: translate to position, uniform scale by size
  glm::mat4 model = glm::translate(glm::mat4(1.0f), position) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(size));

  // Default color white; could add parameter for custom color if desired
  sphereInstances.push_back({model, glm::vec4(1.0f)});
}
uint32_t previousSize = 0;
void DebugDrawer::DrawDebug() {
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_DEBUG_SHAPE_DATA);
  bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_DEBUG_SHAPE_MATRIX_DATA);

  lineDrawCommand.instanceCount = lineInstances.size();
  lineDrawCommand.instanceOffset = 0;

  cubeDrawCommand.instanceCount = cubeInstances.size();
  cubeDrawCommand.instanceOffset = lineInstances.size();

  sphereDrawCommand.instanceCount = sphereInstances.size();
  sphereDrawCommand.instanceOffset =
      cubeDrawCommand.instanceOffset + cubeDrawCommand.instanceCount;

  std::vector<DebugInstance> allInstances;
  allInstances.insert(allInstances.end(), lineInstances.begin(),
                      lineInstances.end());
  allInstances.insert(allInstances.end(), cubeInstances.begin(),
                      cubeInstances.end());
  allInstances.insert(allInstances.end(), sphereInstances.begin(),
                      sphereInstances.end());

  if (previousSize == allInstances.size() && previousSize > 0) {

    bufferManager->UpdateData(instanceBR, allInstances.data(),
                              allInstances.size() * sizeof(DebugInstance));

  } else {
    instanceBR = bufferManager->InsertNewDynamicData(
        allInstances.data(), allInstances.size() * sizeof(DebugInstance),
        TypeFlags::BUFFER_DEBUG_SHAPE_MATRIX_DATA);
  }

  previousSize = allInstances.size();

  glBindVertexArray(m_glDebugVAO);

  // Lines

  // Lines
  if (lineDrawCommand.instanceCount > 0 && lineDrawCommand.indexCount > 0) {
    shaderManager->UseProgramme(lineShader);
    // shaderManager->setInt(lineShader, "vertexOffset",
    //                       lineDrawCommand.vertexOffset);

    glDrawElementsInstancedBaseVertexBaseInstance(
        GL_TRIANGLES, lineDrawCommand.indexCount, GL_UNSIGNED_INT,
        (void *)(lineDrawCommand.indexOffset * sizeof(GLuint)),
        lineDrawCommand.instanceCount,
        lineDrawCommand.vertexOffset, // baseVertex = 0
        lineDrawCommand.instanceOffset);
  }

  // Cubes
  if (cubeDrawCommand.instanceCount > 0 && cubeDrawCommand.indexCount > 0) {
    shaderManager->UseProgramme(meshShader);
    // shaderManager->setInt(meshShader, "vertexOffset",
    //                       cubeDrawCommand.vertexOffset);

    glDrawElementsInstancedBaseVertexBaseInstance(
        GL_TRIANGLES, cubeDrawCommand.indexCount, GL_UNSIGNED_INT,
        (void *)(cubeDrawCommand.indexOffset * sizeof(GLuint)),
        cubeDrawCommand.instanceCount,
        cubeDrawCommand.vertexOffset, // baseVertex = 0
        cubeDrawCommand.instanceOffset);
  }

  // Spheres
  if (sphereDrawCommand.instanceCount > 0 && sphereDrawCommand.indexCount > 0) {
    // shaderManager->setInt(meshShader, "vertexOffset",
    //                       sphereDrawCommand.vertexOffset);
    shaderManager->UseProgramme(meshShader);
    glDrawElementsInstancedBaseVertexBaseInstance(
        GL_TRIANGLES, sphereDrawCommand.indexCount, GL_UNSIGNED_INT,
        (void *)(sphereDrawCommand.indexOffset * sizeof(GLuint)),
        sphereDrawCommand.instanceCount,
        sphereDrawCommand.vertexOffset, // baseVertex = 0
        sphereDrawCommand.instanceOffset);
  }

  lineDrawCommand.instanceCount = 0;
  lineDrawCommand.instanceOffset = 0;

  cubeDrawCommand.instanceCount = 0;
  cubeDrawCommand.instanceOffset = 0;

  sphereDrawCommand.instanceCount = 0;
  sphereDrawCommand.instanceOffset = 0;
  lineInstances.clear();
  cubeInstances.clear();
  sphereInstances.clear();
  bufferManager->ClearBuffer(TypeFlags::BUFFER_DEBUG_SHAPE_MATRIX_DATA);

  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
}

} // namespace eHazGraphics
