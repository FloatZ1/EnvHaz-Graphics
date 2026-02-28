

#include "Utils/Drawing/DebugDrawer.hpp"
#include "DataStructs.hpp"
#include "StaticStack.hpp"
#include "glad/glad.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include <iterator>
#include <optional>
#include <sys/stat.h>
#include <vector>

namespace eHazGraphics {

DebugDrawer *DebugDrawer::s_Instance = nullptr;

DebugDrawer::DebugDrawer(ShaderManager *shaderManager,
                         BufferManager *bufferManager)
    : shaderManager(shaderManager), bufferManager(bufferManager)

{

  s_Instance = this;

  std::string debugInstanceVertexShader = R"glsl(
 //@@start@@ Debug shape vertex shader @@end@@
#version 460 core

layout(location = 0) in vec3 aPos;

// Instance data
struct DebugInstance {
    mat4 model;
    vec4 color;
};
layout(std430, binding = 10) readonly buffer InstanceSSBO {
    DebugInstance instances[];
};

// Camera uniforms
struct VP {
    mat4 view;
    mat4 projection;
};
layout(std430, binding = 5) readonly buffer Camera {
    VP camMats;
};

// Output color to fragment shader
out vec4 vColor;

void main()
{
    // Get correct instance
    uint instanceIndex = gl_BaseInstance + gl_InstanceID;
    vColor = instances[instanceIndex].color;

    // Transform vertex by model, view, projection
    gl_Position =
        camMats.projection *
            camMats.view *                           
            instances[instanceIndex].model *
            vec4(aPos, 1.0);
}  
)glsl";

  static const std::string debugInstanceFragmentShader = R"glsl(

 //@@start@@ Debug shape fragment shader @@end@@
#version 460 core

in vec4 vColor;
out vec4 FragColor;

void main() {
    FragColor = vColor;
}   

)glsl";

  meshShader = shaderManager->CreateShaderProgramme(
      debugInstanceVertexShader, debugInstanceFragmentShader, false);

  std::vector<Vertex> vertices;

  for (int i = 0; i < sizeof(lineQuadVertices) / sizeof(glm::vec3); i++) {

    vertices.push_back({.Position = lineQuadVertices[i]});
  }

  GLuint cubeVertexOffset = vertices.size();
  for (int i = 0; i < sizeof(cubeVertices) / sizeof(glm::vec3); i++) {

    vertices.push_back({.Position = cubeVertices[i]});
  }

  GLuint sphereVertexOffset = vertices.size();

  for (int i = 0; i < sizeof(sphereVertices) / sizeof(glm::vec3); i++) {

    vertices.push_back({.Position = sphereVertices[i]});
  }

  std::vector<GLuint> indices;

  indices.insert(indices.end(), std::begin(lineQuadIndices),
                 std::end(lineQuadIndices));

  indices.insert(indices.end(), std::begin(cubeIndices), std::end(cubeIndices));

  GLuint sphereIndexOffset = indices.size();

  indices.insert(indices.end(), std::begin(sphereIndices),
                 std::end(sphereIndices));

  staticStack = CGLStaticStack(vertices.size() * sizeof(Vertex),
                               indices.size() * sizeof(GLuint), -1);

  staticStack.push_back(vertices.data(), vertices.size() * sizeof(Vertex),
                        indices.data(), indices.size() * sizeof(GLuint));

  GLuint lineIndexCount = sizeof(lineQuadIndices) / sizeof(GLuint);
  GLuint cubeIndexCount = sizeof(cubeIndices) / sizeof(GLuint);
  GLuint sphereIndexCount = sizeof(sphereIndices) / sizeof(GLuint);

  GLuint lineVertexOffset = 0;

  lineCommand.count = lineIndexCount;
  lineCommand.firstIndex = 0;
  lineCommand.baseVertex = 0;

  cubeCommand.count = cubeIndexCount;
  cubeCommand.firstIndex = lineIndexCount;
  cubeCommand.baseVertex = cubeVertexOffset;

  sphereCommand.count = sphereIndexCount;
  sphereCommand.firstIndex = sphereIndexOffset;
  sphereCommand.baseVertex = sphereVertexOffset;
}

void DebugDrawer::SubmitLine(const glm::vec3 &start, const glm::vec3 &end,
                             float thickness, const glm::vec4 &color) {
  glm::vec3 dir = end - start;
  float length = glm::length(dir);
  if (length < 1e-6f)
    return;

  glm::vec3 y = dir / length;

  glm::vec3 helper =
      (glm::abs(y.y) < 0.999f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);

  glm::vec3 x = glm::normalize(glm::cross(helper, y));
  glm::vec3 z = glm::cross(y, x);

  glm::mat4 rotation(1.0f);
  rotation[0] = glm::vec4(x, 0.0f);
  rotation[1] = glm::vec4(y, 0.0f);
  rotation[2] = glm::vec4(z, 0.0f);

  glm::vec3 center = (start + end) * 0.5f;

  glm::vec3 scale(thickness, length * 0.5f, thickness);

  glm::mat4 model = glm::translate(glm::mat4(1.0f), center) * rotation *
                    glm::scale(glm::mat4(1.0f), scale);

  cubeInstances.push_back({model, color});
}

void DebugDrawer::SubmitCube(glm::vec3 position, glm::vec3 halfExtents,
                             glm::vec4 color) {
  // Model matrix: translate to position, scale by half-extents
  glm::mat4 model = glm::translate(glm::mat4(1.0f), position) *
                    glm::scale(glm::mat4(1.0f), halfExtents);

  cubeInstances.push_back({model, color});
}

void DebugDrawer::SubmitSphere(glm::vec3 position, float size) {
  // Model matrix: translate to position, uniform scale by size
  glm::mat4 model = glm::translate(glm::mat4(1.0f), position) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(size));

  // Default color white; could add parameter for custom color if desired
  sphereInstances.push_back({model, glm::vec4(1.0f)});
}
uint32_t previousSize = 0;
void DebugDrawer::DrawDebug(glm::vec3 cameraPos) {
  //  glDisable(GL_DEPTH_TEST);
  //  glDepthMask(GL_FALSE);

  // glDisable(GL_CULL_FACE);
  // bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_DEBUG_SHAPE_DATA_UINT);
  // bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_DEBUG_SHAPE_DATA_FLOAT);

  bufferManager->ClearBuffer(TypeFlags::BUFFER_DEBUG_DRAW_CALL_DATA);
  staticStack.BindBuffer();

  lineCommand.instanceCount = lineInstances.size();
  lineCommand.baseInstance = 0;

  cubeCommand.instanceCount = cubeInstances.size();
  cubeCommand.baseInstance = lineInstances.size();

  sphereCommand.instanceCount = sphereInstances.size();
  sphereCommand.baseInstance =
      cubeCommand.baseInstance + cubeCommand.instanceCount;
  std::vector<DrawElementsIndirectCommand> allCommands = {
      lineCommand, cubeCommand, sphereCommand};

  SBufferRange drawRange = bufferManager->InsertNewDynamicData(
      allCommands.data(),
      allCommands.size() * sizeof(DrawElementsIndirectCommand),
      TypeFlags::BUFFER_DEBUG_DRAW_CALL_DATA);

  glm::vec3 camPos = cameraPos;

  std::sort(cubeInstances.begin(), cubeInstances.end(),
            [&](const DebugInstance &a, const DebugInstance &b) {
              glm::vec3 pa = glm::vec3(a.model[3]);
              glm::vec3 pb = glm::vec3(b.model[3]);

              float da = glm::length(pa - camPos);
              float db = glm::length(pb - camPos);

              // BACK → FRONT (farther first)
              return da > db;
            });

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
  bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_DEBUG_SHAPE_MATRIX_DATA);

  bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_DEBUG_DRAW_CALL_DATA);
  // Lines
  /*if (lineCommand.instanceCount > 0) {
    shaderManager->UseProgramme(lineShader);

    GLintptr offset = 0 * sizeof(DrawElementsIndirectCommand);

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void *)offset,
                                1, 0);
  }*/

  // Cubes
  if (cubeCommand.instanceCount > 0) {

    shaderManager->UseProgramme(meshShader);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //  glEnable(GL_DEPTH_TEST);
    //  glDepthMask(GL_FALSE);
    GLintptr offset = sizeof(DrawElementsIndirectCommand);

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void *)offset,
                                1, 0);

    // glDepthMask(GL_TRUE);

    glDisable(GL_BLEND);
  }

  // Spheres
  if (sphereCommand.instanceCount > 0) {

    shaderManager->UseProgramme(meshShader);
    GLintptr offset = 2 * sizeof(DrawElementsIndirectCommand);

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void *)offset,
                                1, 0);
  }

  lineCommand.instanceCount = 0;
  lineCommand.baseInstance = 0;

  cubeCommand.instanceCount = 0;
  cubeCommand.baseInstance = 0;

  sphereCommand.instanceCount = 0;
  sphereCommand.baseInstance = 0;
  lineInstances.clear();
  cubeInstances.clear();
  sphereInstances.clear();
  // bufferManager->ClearBuffer(TypeFlags::BUFFER_DEBUG_SHAPE_MATRIX_DATA);

  //  glDepthMask(GL_TRUE);
  //  glEnable(GL_DEPTH_TEST);

  // glEnable(GL_CULL_FACE);
}

} // namespace eHazGraphics
