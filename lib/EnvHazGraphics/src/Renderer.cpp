#include "Renderer.hpp"
#include "Animation/AnimatedModelManager.hpp"
#include "BitFlags.hpp"
#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include "FrameBuffers/FrameBuffer.hpp"
#include "MaterialManager.hpp"
#include "MeshManager.hpp"
#include "RenderQueue.hpp"
#include "ShaderManager.hpp"
#include "Window.hpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <glad/glad.h>
#include <iostream>
#include <memory>
#include <vector>
// #define EHAZ_DEBUG

#ifdef EHAZ_DEBUG

void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar *message, const void *userParam) {
  const char *src = "";
  switch (source) {
  case GL_DEBUG_SOURCE_API:
    src = "API";
    break;
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    src = "Window System";
    break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    src = "Shader Compiler";
    break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    src = "Third Party";
    break;
  case GL_DEBUG_SOURCE_APPLICATION:
    src = "Application";
    break;
  case GL_DEBUG_SOURCE_OTHER:
    src = "Other";
    break;
  }

  const char *typ = "";
  switch (type) {
  case GL_DEBUG_TYPE_ERROR:
    typ = "Error";
    break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    typ = "Deprecated";
    break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    typ = "Undefined";
    break;
  case GL_DEBUG_TYPE_PORTABILITY:
    typ = "Portability";
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    typ = "Performance";
    break;
  case GL_DEBUG_TYPE_MARKER:
    typ = "Marker";
    break;
  case GL_DEBUG_TYPE_PUSH_GROUP:
    typ = "Push Group";
    break;
  case GL_DEBUG_TYPE_POP_GROUP:
    typ = "Pop Group";
    break;
  case GL_DEBUG_TYPE_OTHER:
    typ = "Other";
    break;
  }

  const char *sev = "";
  switch (severity) {
  case GL_DEBUG_SEVERITY_HIGH:
    sev = "HIGH";
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    sev = "MEDIUM";
    break;
  case GL_DEBUG_SEVERITY_LOW:
    sev = "LOW";
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    sev = "NOTIFICATION";
    break;
  }

  SDL_Log("GL CALLBACK: %s type = %s, severity = %s, message = %s", src, typ,
          sev, message);
}

#endif

namespace eHazGraphics {

std::unique_ptr<Window> Renderer::p_window = nullptr;
std::unique_ptr<Renderer> Renderer::r_instance = nullptr;
std::unique_ptr<ShaderManager> Renderer::p_shaderManager = nullptr;
std::unique_ptr<MaterialManager> Renderer::p_materialManager = nullptr;
std::unique_ptr<MeshManager> Renderer::p_meshManager = nullptr;
std::unique_ptr<RenderQueue> Renderer::p_renderQueue = nullptr;
std::unique_ptr<BufferManager> Renderer::p_bufferManager = nullptr;
std::unique_ptr<AnimatedModelManager> Renderer::p_AnimatedModelManager =
    nullptr;

bool Renderer::Initialize(int width, int height, std::string tittle,
                          bool fullscreen) {
  r_instance.reset(this);
  p_window = std::make_unique<Window>();

  bool success{true};

  if (SDL_Init(SDL_INIT_VIDEO) == false) {
    SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
    success = false;
  }

  if (p_window->Create(width, height, fullscreen, tittle)) {
    SDL_Log("Window created successfully.\n");
    if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
      SDL_Log("GLAD INITIALIZED SUCESSFULLY!");
      glViewport(0, 0, p_window->GetWidth(), p_window->GetHeight());
      glClearColor(0.1f, 0.5f, 0.7f, 1.0f);
    } else {
      SDL_Log("Failed to initialize GLAD");
    }
  }

#ifdef EHAZ_DEBUG

  // Check if debug output is available (requires OpenGL 4.3+ or
  // ARB_debug_output)
  int flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
  if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // synchronous callback
    glDebugMessageCallback(GLDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                          GL_TRUE);

    SDL_Log("OpenGL debug output enabled!");
  } else {
    SDL_Log("OpenGL debug context not available!");
  }

#endif

  bool hasSSBO = false;
  bool hasCompute = false;
  bool hasBindless = false;

  GLint numExtensions = 0;
  glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

  for (GLint i = 0; i < numExtensions; i++) {
    const char *ext = (const char *)glGetStringi(GL_EXTENSIONS, i);
    if (strcmp(ext, "GL_ARB_shader_storage_buffer_object") == 0) {
      hasSSBO = true;
    }
    if (strcmp(ext, "GL_ARB_compute_shader") == 0) {
      hasCompute = true;
    }
    if (strcmp(ext, "GL_ARB_bindless_texture") == 0) {
      hasBindless = true;
    }
  }

  if (hasSSBO) {
    SDL_Log("SSBOs supported via GL_ARB_shader_storage_buffer_object");
  } else {
    SDL_Log("SSBOs NOT supported on this driver!");
  }

  if (hasCompute) {
    SDL_Log("Compute shaders supported via GL_ARB_compute_shader");
  } else {
    SDL_Log("Compute shaders NOT supported on this driver!");
  }

  if (hasCompute) {
    SDL_Log("Bindless texture supported via GL_ARB_bindless_texture");
  } else {
    SDL_Log("Bindless texture NOT supported on this driver!");
  }

  if (GLAD_GL_ARB_bindless_texture) {
    std::cout << "Bindless textures supported!" << std::endl;
  } else {
    std::cerr << "GL_ARB_bindless_texture not supported on this system!"
              << std::endl;
  }

  SetViewport(p_window->GetWidth(), p_window->GetHeight());

  p_shaderManager = std::make_unique<ShaderManager>();
  p_materialManager = std::make_unique<MaterialManager>();
  p_meshManager = std::make_unique<MeshManager>();
  p_renderQueue = std::make_unique<RenderQueue>();
  p_bufferManager = std::make_unique<BufferManager>();
  p_AnimatedModelManager = std::make_unique<AnimatedModelManager>();

  p_bufferManager->Initialize();
  p_shaderManager->Initialize();
  p_meshManager->Initialize(p_bufferManager.get());
  p_AnimatedModelManager->Initialize(p_bufferManager.get());

  p_materialManager->Initialize();

  p_renderQueue->Initialize(p_bufferManager.get());
  std::string ScreenRenderVS =
      "//@@start@@ ScreenRenderVS shader @@end@@\n"
      "#version 460 core\n"
      "const vec2 verts[3] = vec2[](\n"
      "    vec2(-1.0, -1.0),\n"
      "    vec2(3.0, -1.0),\n"
      "    vec2(-1.0, 3.0)\n"
      ");\n"
      "void main() {\n"
      "    gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);\n"
      "}\n";

  std::string ScreenRenderFS =
      "//@@start@@ ScreenRenderFS shader @@end@@\n"
      "#version 460 core\n"
      "layout(binding = 0) uniform sampler2D u_ScreenTex;\n"
      "out vec4 FragColor;\n"
      "void main() {\n"
      "    vec2 uv = gl_FragCoord.xy / vec2(textureSize(u_ScreenTex, 0));\n"
      "    FragColor = vec4(vec3(1.0) - texture(u_ScreenTex, uv).rgb,\n"
      "                     texture(u_ScreenTex, uv).a);\n"
      "}\n";

  auto dis = p_shaderManager->CreateShaderProgramme(ScreenRenderVS,
                                                    ScreenRenderFS, false);

  mainFBO.SetDisplayShader(dis);

  std::vector<RenderTexture2D_Spec> colors = {
      {p_window->GetWidth(), p_window->GetHeight(),
       GL_RGBA16F} // HDR color buffer
  };

  RenderTexture2D_Spec depth;
  depth.width = colors[0].width;
  depth.height = colors[0].height;
  depth.internalFormat = GL_DEPTH_COMPONENT24;
  depth.format = GL_DEPTH_COMPONENT; // important
  depth.type = GL_FLOAT;
  mainFBO.Create(colors, depth);

  DefaultFrameBuffer();
  // glBindFramebuffer(GL_FRAMEBUFFER, 0);

  assert(p_shaderManager && "ShaderManager is not initialized");
  assert(p_window && "Window is not initialized");

  return success;
  std::cout << "piss\n\n :)))";
}

void Renderer::SubmitAnimatedModel(std::shared_ptr<AnimatedModel> &model,
                                   glm::mat4 position) {

  std::vector<SBufferRange> instanceRanges;
  std::vector<InstanceData> instances;

  for (auto &mesh : model->GetMeshIDs()) {

    VertexIndexInfoPair range;

    const Mesh &m_mesh = p_AnimatedModelManager->GetMesh(mesh);
    if (m_mesh.isResident() == false) {
      const auto &vertexPair = m_mesh.GetVertexData();
      const auto &indexPair = m_mesh.GetIndexData();

      range = p_bufferManager->InsertNewStaticData(
          vertexPair.first, vertexPair.second, indexPair.first,
          indexPair.second,
          TypeFlags::BUFFER_STATIC_MESH_DATA); // TODO: add vertex pulling for
                                               // the animated meshes

      p_AnimatedModelManager->AddMeshLocation(mesh, range);
      p_AnimatedModelManager->SetMeshResidency(mesh, true);

    } else {
      range = p_AnimatedModelManager->GetMeshLocation(mesh);
    }

    auto &animator =
        p_AnimatedModelManager->GetAnimator(model->GetAnimatorID());
    // TODO: ADD CHECKS FOR NULLOPT and for the static asw
    size_t animatorMatrixOffset =
        p_bufferManager->GetAllocation(animator->GetGPULocation())->offset;

    uint32_t matID = animatorMatrixOffset / sizeof(glm::mat4);

    unsigned int numJoints = model->GetSkeleton()->m_Joints.size();

    unsigned int jointLocation = animatorMatrixOffset / sizeof(glm::mat4);

    InstanceData instData{position, model->GetMaterialID(), matID, numJoints,
                          jointLocation};

    SBufferRange instanceData = p_bufferManager->InsertNewDynamicData(
        &instData, sizeof(InstanceData), TypeFlags::BUFFER_INSTANCE_DATA);

    size_t instanceOffset =
        p_bufferManager->GetAllocation(instanceData)->offset;

    size_t instanceID = instanceOffset / sizeof(InstanceData);

    instanceRanges.push_back(instanceData);
    instances.push_back(instData);

    int cmdID = p_renderQueue->CreateRenderCommand(range, true, instanceID,
                                                   m_mesh.GetInstanceCount(),
                                                   m_mesh.GetShaderID());
  }

  p_AnimatedModelManager->AddSubmittedModel(model);
  model->AddInstances(instances, instanceRanges);
}

// Model& model , TypeFlags dataType
void Renderer::SubmitStaticModel(std::shared_ptr<Model> &model,
                                 glm::mat4 position, TypeFlags dataType) {

  // p_bufferManager->ClearBuffer(dataType);
  std::vector<SBufferRange> instanceRanges;
  std::vector<InstanceData> instances;

  for (auto &mesh : model->GetMeshIDs()) {

    VertexIndexInfoPair range;

    const Mesh &m_mesh = p_meshManager->GetMesh(mesh);
    if (m_mesh.isResident() == false) {
      const auto &vertexPair = m_mesh.GetVertexData();
      const auto &indexPair = m_mesh.GetIndexData();

      range = p_bufferManager->InsertNewStaticData(
          vertexPair.first, vertexPair.second, indexPair.first,
          indexPair.second, dataType);
      p_meshManager->AddMeshLocation(mesh, range);
      p_meshManager->SetMeshResidency(mesh, true);
    } else {
      range = p_meshManager->GetMeshLocation(mesh);
    }

    // TODO: Get the instance data from the model and create the necessary
    // render commands

    glm::mat4 meshMat = p_meshManager->GetMeshTransform(mesh);

    SBufferRange matLocation;

    if (p_meshManager->ContainsTransformRange(mesh)) {

      matLocation = p_meshManager->GetTransformBufferRange(mesh);
    } else {

      matLocation = p_bufferManager->InsertNewDynamicData(
          &meshMat, sizeof(meshMat), TypeFlags::BUFFER_STATIC_MATRIX_DATA);
      p_meshManager->AddTransformRange(mesh, matLocation);
    }

    size_t matOffset = p_bufferManager->GetAllocation(matLocation)->offset;

    uint32_t matID = matOffset / sizeof(glm::mat4);

    InstanceData instData{position, model->GetMaterialID(), matID};

    SBufferRange instanceData = p_bufferManager->InsertNewDynamicData(
        &instData, sizeof(InstanceData), TypeFlags::BUFFER_INSTANCE_DATA);

    size_t instanceOffset =
        p_bufferManager->GetAllocation(instanceData)->offset;

    size_t instanceID = instanceOffset / sizeof(InstanceData);

    instanceRanges.push_back(instanceData);
    instances.push_back(instData);

    int cmdID = p_renderQueue->CreateRenderCommand(range, true, instanceID,
                                                   m_mesh.GetInstanceCount(),
                                                   m_mesh.GetShaderID());
  }

  p_meshManager->AddSubmittedModel(model);
  // model->SetInstances(instances, instanceRanges);
  model->AddInstances(instances, instanceRanges);
}
SBufferRange Renderer::SubmitDynamicData(const void *data, size_t dataSize,
                                         TypeFlags dataType) {
  SBufferRange rt;

  rt = p_bufferManager->InsertNewDynamicData(data, dataSize, dataType);
  rt.dataType = dataType;
  return rt;
  // only for new data
}

void Renderer::PollInputEvents() { SDL_PollEvent(&events); }

void Renderer::RenderFrame(std::vector<DrawRange> DrawOrder) {

  p_bufferManager->EndWritting();
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  glEnable(GL_DEPTH_TEST);
  // glDisable(GL_CULL_FACE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Bind the static mesh buffer
  p_bufferManager->BindStaticBuffer(TypeFlags::BUFFER_STATIC_MESH_DATA);

  // Bind the dynamic draw command buffer
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_DRAW_CALL_DATA);
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_CAMERA_DATA);
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_INSTANCE_DATA);
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_TEXTURE_DATA);
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_STATIC_MATRIX_DATA);
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_ANIMATION_DATA);

  if (!p_shaderManager || !p_window) {
    SDL_Log("RenderFrame called with uninitialized managers!");
    return;
  }

  for (const auto &range : DrawOrder) {
    p_shaderManager->UseProgramme(range.shader);
    GLintptr offset = range.startIndex * sizeof(DrawElementsIndirectCommand);

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void *)offset,
                                range.count, 0);
  }

  //  SDL_GL_SwapWindow(p_window->GetWindowPtr());

  p_bufferManager->BeginWritting();
  ClearRenderCommandBuffer();
  p_renderQueue->ClearDynamicCommands();
  p_renderQueue->ClearStaticCommnads();
  p_meshManager->ClearSubmittedModelInstances();
  p_AnimatedModelManager->ClearSubmittedModelInstances();
}

void Renderer::UpdateRenderer(float deltatime) {
  PollInputEvents();
  if (events.type == SDL_EVENT_QUIT)
    shouldQuit = true;

  p_window->Update();
  //  p_bufferManager->UpdateManager();
  p_meshManager->Update();
  p_AnimatedModelManager->Update(deltatime);
}

void Renderer::Destroy() {

  p_meshManager->Destroy();
  p_renderQueue->Destroy();
  //  bufferManager.Destroy();
  p_bufferManager->Destroy();
  p_shaderManager->Destroy();
  p_materialManager->Destroy();
}

void Renderer::UpdateDynamicData(const SBufferRange &location, const void *data,
                                 const size_t size) {

  p_bufferManager->UpdateData(location, data, size);
}

void Renderer::ClearRenderCommandBuffer() {

  p_bufferManager->ClearBuffer(TypeFlags::BUFFER_INSTANCE_DATA);
}

void Renderer::SetViewport(int width, int height) {

  glViewport(0, 0, width, height);
  vp_height = height;
  vp_width = width;
}
void Renderer::SetFrameBuffer(const FrameBuffer &fbo) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo.GetFBO());
  SetViewport(fbo.GetWidth(), fbo.GetHeight());
}

void Renderer::DefaultFrameBuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  SetViewport(p_window->GetWidth(), p_window->GetHeight());
}

} // namespace eHazGraphics
