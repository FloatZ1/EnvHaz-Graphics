#include "Renderer.hpp"
#include "Animation/AnimatedModelManager.hpp"
#include "BitFlags.hpp"
#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include "FrameBuffers/FrameBuffer.hpp"
#include "FrameBuffers/HDR_Buffer.hpp"
#include "FrameBuffers/HDR_shader_source.hpp"
#include "FrameBuffers/RenderTexture.hpp"
#include "FrameBuffers/geometry_buffer.hpp"
#include "MaterialManager.hpp"
#include "MeshManager.hpp"
#include "Model.hpp"
#include "RenderQueue.hpp"
#include "ShaderManager.hpp"
#include "Utils/Drawing/DebugDrawer.hpp"
#include "Utils/SingleDrawBuffer.hpp"
#include "Window.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/matrix.hpp"
#include "glm/simd/platform.h"
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
    return;
    break;
  }

  SDL_Log("GL CALLBACK: %s type = %s, severity = %s, message = %s", src, typ,
          sev, message);
}

#endif

namespace eHazGraphics {
struct SCameraData {
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);
  glm::mat4 inverseViewProjectionNoTranslation = glm::mat4(1.0f);
  glm::mat4 inverseViewProjection = glm::mat4(1.0f);
};

std::unique_ptr<Window> Renderer::p_window = nullptr;
std::unique_ptr<Renderer> Renderer::r_instance = nullptr;
std::unique_ptr<ShaderManager> Renderer::p_shaderManager = nullptr;
std::unique_ptr<MaterialManager> Renderer::p_materialManager = nullptr;
std::unique_ptr<MeshManager> Renderer::p_meshManager = nullptr;
std::unique_ptr<RenderQueue> Renderer::p_renderQueue = nullptr;
std::unique_ptr<BufferManager> Renderer::p_bufferManager = nullptr;
std::unique_ptr<AnimatedModelManager> Renderer::p_AnimatedModelManager =
    nullptr;

std::unique_ptr<DebugDrawer> Renderer::p_debugDrawer = nullptr;
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
  // glEnable(GL_FRAMEBUFFER_SRGB);

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

  const char *version = (const char *)glGetString(GL_VERSION);

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

  p_debugDrawer = std::make_unique<DebugDrawer>(p_shaderManager.get(),
                                                p_bufferManager.get());

  /*  m_sdbSkyModelSide1_Buffer =
        std::make_unique<eHazGraphics_Utils::SingleDrawBuffer>();
    m_sdbSkyModelSide2_Buffer =
        std::make_unique<eHazGraphics_Utils::SingleDrawBuffer>();
    m_sdbSkyModelTop_Buffer =
        std::make_unique<eHazGraphics_Utils::SingleDrawBuffer>();
     */

  m_sdbSkyModelSide1_Buffer.Initialize();
  m_sdbSkyModelSide2_Buffer.Initialize();
  m_sdbSkyModelTop_Buffer.Initialize();

  SCameraData cameraData{ViewMatrix, ProjectionMatrix};

  m_brCameraData = p_bufferManager->InsertNewDynamicData(
      &cameraData, sizeof(cameraData), TypeFlags::BUFFER_CAMERA_DATA);

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

  m_scidHDRshader = p_shaderManager->CreateShaderProgramme(
      g_strHDRVertexSourceCode, g_strHDRFragmentSourceCode, false);

  m_scidToneShader = p_shaderManager->CreateShaderProgramme(
      g_strToneVertexSourceCode, g_strToneFragmentSourceCode, false);

  mainFBO.SetDisplayShader(dis);

  std::vector<RenderTexture2D_Spec> colors = {
      {p_window->GetWidth(), p_window->GetHeight(), 1,
       GL_RGBA16F} // HDR color buffer
  };

  m_gbGeometryBuffer = new CGeometryBuffer(
      p_window->GetWidth(), p_window->GetHeight(), m_scidHDRshader);
  m_bHDRBuffer = new CHDRBuffer(p_window->GetWidth(), p_window->GetHeight());

  RenderTexture2D_Spec depth;
  depth.layers = 1;
  depth.width = colors[0].width;
  depth.height = colors[0].height;
  depth.internalFormat = GL_DEPTH_COMPONENT24;
  depth.format = GL_DEPTH_COMPONENT; // important
  depth.type = GL_UNSIGNED_INT;
  depth.target = GL_TEXTURE_2D;
  mainFBO.Create(colors, depth);

  RenderTexture2D_Spec shadowDepths;
  shadowDepths.layers = 4;
  shadowDepths.internalFormat = GL_DEPTH_COMPONENT32F;
  shadowDepths.width = m_uiShadowTexWidth;
  shadowDepths.height = m_uiShadowTexHeight;
  shadowDepths.format = GL_DEPTH_COMPONENT;
  shadowDepths.type = GL_FLOAT;
  shadowDepths.target = GL_TEXTURE_2D_ARRAY;
  shadowDepths.enableCompare = true;

  m_fbShadowCascadeBuffer.Create({}, shadowDepths);

  m_fbShadowCascadeBuffer.SetDisplayShader(m_scidCSMshader);

  DefaultFrameBuffer();

  // glBindFramebuffer(GL_FRAMEBUFFER, 0);

  assert(p_shaderManager && "ShaderManager is not initialized");
  assert(p_window && "Window is not initialized");

  return success;
}

void Renderer::SubmitAnimatedModel(ModelID modelID, glm::mat4 position,
                                   uint32_t materialID,
                                   ShaderComboID usedShader) {

  std::shared_ptr<AnimatedModel> model =
      p_AnimatedModelManager->GetModel(modelID);

  std::vector<SBufferRange> instanceRanges;
  std::vector<InstanceData> instances;

  for (auto &mesh : model->GetMeshIDs()) {

    VertexIndexInfoPair range;

    const Mesh &m_mesh = p_AnimatedModelManager->GetMesh(mesh);
    if (m_mesh.isResident() == false) {
      const auto &vertexPair = m_mesh.GetVertexData();
      const auto &indexPair = m_mesh.GetIndexData();
      WaitForGPU();
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
    size_t animatorMatrixOffset; // =
    if (p_bufferManager->GetAllocation(animator->GetGPULocation()) ==
        std::nullopt) {

      animatorMatrixOffset = 0;

    } else {

      animatorMatrixOffset =
          p_bufferManager->GetAllocation(animator->GetGPULocation())->offset;
    }

    uint32_t matID = animatorMatrixOffset / sizeof(glm::mat4);

    unsigned int numJoints = model->GetSkeleton()->m_Joints.size();

    unsigned int jointLocation = animatorMatrixOffset / sizeof(glm::mat4);

    InstanceData instData{position, materialID, matID, numJoints,
                          jointLocation};

    SBufferRange instanceData = p_bufferManager->InsertNewDynamicData(
        &instData, sizeof(InstanceData), TypeFlags::BUFFER_INSTANCE_DATA);

    size_t instanceOffset =
        p_bufferManager->GetAllocation(instanceData)->offset;

    size_t instanceID = instanceOffset / sizeof(InstanceData);

    instanceRanges.push_back(instanceData);
    instances.push_back(instData);

    int cmdID = p_renderQueue->CreateRenderCommand(
        range, true, instanceID, m_mesh.GetInstanceCount(), usedShader);
  }

  p_AnimatedModelManager->AddSubmittedModel(model);
  model->AddInstances(instances, instanceRanges);
}

// Model& model , TypeFlags dataType
void Renderer::SubmitStaticModel(ModelID modelID, glm::mat4 position,
                                 uint32_t materialID, ShaderComboID usedShader,
                                 TypeFlags dataType) {

  std::shared_ptr<Model> model = p_meshManager->GetModel(modelID);
  std::vector<SBufferRange> instanceRanges;
  std::vector<InstanceData> instances;

  for (auto &mesh : model->GetMeshIDs()) {

    VertexIndexInfoPair range;

    const Mesh &m_mesh = p_meshManager->GetMesh(mesh);
    if (m_mesh.isResident() == false) {
      const auto &vertexPair = m_mesh.GetVertexData();
      const auto &indexPair = m_mesh.GetIndexData();
      WaitForGPU();
      range = p_bufferManager->InsertNewStaticData(
          vertexPair.first, vertexPair.second, indexPair.first,
          indexPair.second, dataType);
      p_meshManager->AddMeshLocation(mesh, range);
      p_meshManager->SetMeshResidency(mesh, true);

      // p_meshManager->AddTransformRange(mesh,
      // p_bufferManager->InsertNewDynamicData(&m_mesh.GetRelativeMatrix(),
      // sizeof(glm::mat4),
      //     TypeFlags::BUFFER_STATIC_MATRIX_DATA));

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

    InstanceData instData{position, materialID, matID};

    SBufferRange instanceData = p_bufferManager->InsertNewDynamicData(
        &instData, sizeof(InstanceData), TypeFlags::BUFFER_INSTANCE_DATA);

    size_t instanceOffset =
        p_bufferManager->GetAllocation(instanceData)->offset;

    size_t instanceID = instanceOffset / sizeof(InstanceData);

    instanceRanges.push_back(instanceData);
    instances.push_back(instData);

    int cmdID = p_renderQueue->CreateRenderCommand(
        range, true, instanceID, m_mesh.GetInstanceCount(), usedShader);
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
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_LIGHT_DATA);
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_GI_PROBE_DATA);

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

  // p_bufferManager->WaitForBuffer(TypeFlags::BUFFER_DRAW_CALL_DATA);
  // p_bufferManager->ClearBuffer(TypeFlags::BUFFER_DRAW_CALL_DATA);

  //  SDL_GL_SwapWindow(p_window->GetWindowPtr());

  p_bufferManager->BeginWritting();

  ClearRenderCommandBuffer();
  p_renderQueue->ClearDynamicCommands();
  p_renderQueue->ClearStaticCommnads();
  p_meshManager->ClearSubmittedModelInstances();
  p_bufferManager->ClearBuffer(TypeFlags::BUFFER_ANIMATION_DATA);

  p_bufferManager->ClearBuffer(TypeFlags::BUFFER_STATIC_MATRIX_DATA);
  p_AnimatedModelManager->ClearSubmittedModelInstances();

  // p_debugDrawer->DrawDebug(cameraPosition);
}

void Renderer::UpdateRenderer(float deltatime) {

  glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(ViewMatrix));
  glm::mat4 inverseViewProjectionNoTranslation =
      glm::inverse(ProjectionMatrix * viewNoTranslation);

  glm::mat4 inverseViewProjection = glm::inverse(ProjectionMatrix * ViewMatrix);

  SCameraData l_cdCameraMatrices{ViewMatrix, ProjectionMatrix,
                                 inverseViewProjectionNoTranslation,
                                 inverseViewProjection};
  Renderer::r_instance->UpdateDynamicData(m_brCameraData, &l_cdCameraMatrices,
                                          sizeof(l_cdCameraMatrices));

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

void Renderer::UpdateDynamicData(SBufferRange &location, const void *data,
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
  fb_width = fbo.GetWidth();
  fb_height = fbo.GetHeight();
  m_uiCurrentBoundFBO = fbo.GetFBO();
}

void Renderer::DefaultFrameBuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  SetViewport(p_window->GetWidth(), p_window->GetHeight());
  fb_width = p_window->GetWidth();
  fb_width = p_window->GetHeight();
  m_uiCurrentBoundFBO = 0;
}

void Renderer::RenderLightingPass() {
  // glDepthMask(GL_FALSE);
  BindHDRBuffer();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  BindGeometryBufferTextures(4);
  p_shaderManager->UseProgramme(m_scidHDRshader);
  p_shaderManager->setInt(m_scidHDRshader, "numLights", m_uiNumLights);
  p_shaderManager->setVec3(m_scidHDRshader, "u_AmbientSky",
                           GetAmbientSkyColor());
  p_shaderManager->setInt(m_scidHDRshader, "numProbes", m_uiNumGI_probes);
  for (int i = 0; i < 4; i++) {
    p_shaderManager->s_Instance->setMat4(
        m_scidHDRshader, "u_LightMatrices[" + std::to_string(i) + "]",
        m_arrShadowMatrices[i]);
  }
  for (int i = 0; i < 4; i++) {
    p_shaderManager->s_Instance->setFloat(
        m_scidHDRshader, "u_CascadeEnds[" + std::to_string(i) + "]",
        u_CascadeEnds[i]);
  }

  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D_ARRAY,
                m_fbShadowCascadeBuffer.GetDepthTexture().GetTextureID());
  p_shaderManager->setInt(m_scidHDRshader, "u_ShadowMap", 7);

  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
}

void Renderer::RenderSkyPass() {
  BindHDRBuffer();
  // glClear(GL_COLOR_BUFFER_BIT);

  // =========================
  // 🌌 ATMOSPHERE (fullscreen)
  // =========================

  m_gbGeometryBuffer->BindDepth(4);
  m_bHDRBuffer->BindColor();

  p_shaderManager->UseProgramme(m_scidSkyboxShader);

  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  p_shaderManager->setVec2(m_scidSkyboxShader, "resolution",
                           {(float)fb_width, (float)fb_height});

  p_shaderManager->setVec3(m_scidSkyboxShader, "BETA_RAYLEIGH",
                           m_v3BetaRayleigh * 1e-6f);
  p_shaderManager->setVec3(m_scidSkyboxShader, "BETA_MIE", m_v3BetaMie * 1e-6f);
  p_shaderManager->setVec3(m_scidSkyboxShader, "BETA_OZONE",
                           m_v3BetaOzone * 1e-6f);
  p_shaderManager->setVec3(m_scidSkyboxShader, "dirToSun", m_v3SunDirection);

  p_shaderManager->setFloat(m_scidSkyboxShader, "RAYLEIGH_MULTIPLIER",
                            m_fRayLeighScale);
  p_shaderManager->setFloat(m_scidSkyboxShader, "MIE_MULTIPLIER", m_fMieScale);
  p_shaderManager->setFloat(m_scidSkyboxShader, "light_exposure",
                            m_fLightExposure);
  p_shaderManager->setFloat(m_scidSkyboxShader, "solar_brightness",
                            m_fSolarBrightness);

  p_shaderManager->setFloat(m_scidSkyboxShader, "sunSize", m_fSunSize);
  p_shaderManager->setVec3(m_scidSkyboxShader, "sunColor", m_v3SunColor);

  glDrawArrays(GL_TRIANGLES, 0, 3);

  // =========================
  // ☁️ SKYDOME (always visible, blended)
  // =========================
  // 🔥 do NOT write depth

  glm::mat4 skydomeMat = glm::mat4(1.0f);
  skydomeMat = glm::translate(skydomeMat, cameraPosition);
  skydomeMat = glm::scale(skydomeMat, glm::vec3(m_fSkyModelScale));

  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gbGeometryBuffer->GetFBO().GetFBO());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_bHDRBuffer->GetFBO().GetFBO());

  glBlitFramebuffer(0, 0, fb_width, fb_height, 0, 0, fb_width, fb_height,
                    GL_DEPTH_BUFFER_BIT, GL_NEAREST);

  p_shaderManager->UseProgramme(m_scidSkyModelShader);
  // 🔥 Re-apply state AFTER UseProgramme
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL); // important for "always render behind"
  glDepthMask(GL_FALSE);
  // glEnable(GL_DEPTH_TEST);

  /* glDisable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   glDepthMask(GL_FALSE);  */

  // --- Draw all parts ---
  auto drawPart = [&](eHazGraphics_Utils::CSingleDrawBuffer &buffer,
                      int matID) {
    p_shaderManager->setMat4(m_scidSkyModelShader, "localMat",
                             buffer.GetRelativeMatrix());
    p_shaderManager->setMat4(m_scidSkyModelShader, "modelMat", skydomeMat);
    p_shaderManager->setInt(m_scidSkyModelShader, "matID", matID);
    buffer.Draw();
  };

  drawPart(m_sdbSkyModelSide2_Buffer, m_matSkyModelSide2);
  drawPart(m_sdbSkyModelTop_Buffer, m_matSkyModelTop);
  drawPart(m_sdbSkyModelSide1_Buffer, m_matSkyModelSide1);

  // =========================
  // 🎬 TONEMAP
  // =========================
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE); // restore default
  glDepthFunc(GL_LESS);
}

void Renderer::RenderHDRToScreen() {

  p_shaderManager->UseProgramme(m_scidToneShader);
  glDisable(GL_DEPTH_TEST);
  m_bHDRBuffer->BindColor(0);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  // =========================
  // 🔄 RESTORE DEFAULTS
  // =========================
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);

  glClear(GL_DEPTH_BUFFER_BIT);
}
void printMat4(const glm::mat4 &m) {
  std::cout << "------------------------------------------" << std::endl;
  for (int i = 0; i < 4; ++i) {
    std::cout << "| ";
    for (int j = 0; j < 4; ++j) {
      // Accessing [column][row] to print row-by-row
      std::cout << std::setw(10) << std::fixed << std::setprecision(4)
                << m[j][i] << " ";
    }
    std::cout << " |" << std::endl;
  }
  std::cout << "------------------------------------------" << std::endl;
}
void Renderer::RenderShadowMapTextures(std::vector<DrawRange> DrawOrder) {

  int pfb_height = fb_height, pfb_width = fb_width;
  GLuint prevBuffer = m_uiCurrentBoundFBO;

  auto &shadowMatrices = m_arrShadowMatrices;
  SetFrameBuffer(m_fbShadowCascadeBuffer);
  glClear(GL_DEPTH_BUFFER_BIT);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  /*  printMat4(shadowMatrices[0]);

    printMat4(shadowMatrices[1]);

    printMat4(shadowMatrices[2]);

    printMat4(shadowMatrices[3]); */

  p_bufferManager->BindStaticBuffer(TypeFlags::BUFFER_STATIC_MESH_DATA);
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_DRAW_CALL_DATA);
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_CAMERA_DATA);
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_INSTANCE_DATA);
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_TEXTURE_DATA);
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_STATIC_MATRIX_DATA);
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_ANIMATION_DATA);
  p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_LIGHT_DATA);

  p_shaderManager->UseProgramme(m_scidCSMshader);

  p_shaderManager->setMat4(m_scidCSMshader, "u_LightMatrices[0]",
                           shadowMatrices[0]);

  p_shaderManager->setMat4(m_scidCSMshader, "u_LightMatrices[1]",
                           shadowMatrices[1]);
  p_shaderManager->setMat4(m_scidCSMshader, "u_LightMatrices[2]",
                           shadowMatrices[2]);
  p_shaderManager->setMat4(m_scidCSMshader, "u_LightMatrices[3]",
                           shadowMatrices[3]);
  uint32_t count = 0;
  for (const auto &range : DrawOrder) {

    count += range.count;
  }

  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void *)0, count,
                              0);

  glCullFace(GL_BACK);
  glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

  ClearRenderCommandBuffer();
  p_renderQueue->ClearDynamicCommands();
  p_renderQueue->ClearStaticCommnads();
  p_meshManager->ClearSubmittedModelInstances();

  p_bufferManager->ClearBuffer(TypeFlags::BUFFER_STATIC_MATRIX_DATA);
  p_AnimatedModelManager->ClearSubmittedModelInstances();

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  glBindFramebuffer(GL_FRAMEBUFFER, prevBuffer);
  SetViewport(pfb_width, pfb_height);
  m_uiCurrentBoundFBO = prevBuffer;
  fb_width = pfb_width;
  fb_height = pfb_height;
}

void Renderer::RenderOnCurrentFrame(std::vector<DrawRange> DrawOrder) {

  /*  p_bufferManager->EndWritting();

    // glDisable(GL_CULL_FACE);
    // Bind the static mesh buffer
    p_bufferManager->BindStaticBuffer(TypeFlags::BUFFER_STATIC_MESH_DATA);

    // Bind the dynamic draw command buffer
    p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_DRAW_CALL_DATA);
    p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_CAMERA_DATA);
    p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_INSTANCE_DATA);
    p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_TEXTURE_DATA);
    p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_STATIC_MATRIX_DATA);
    p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_ANIMATION_DATA);
    p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_LIGHT_DATA);

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

    // p_bufferManager->WaitForBuffer(TypeFlags::BUFFER_DRAW_CALL_DATA);
    // p_bufferManager->ClearBuffer(TypeFlags::BUFFER_DRAW_CALL_DATA);

    //  SDL_GL_SwapWindow(p_window->GetWindowPtr());

    p_bufferManager->BeginWritting();

    ClearRenderCommandBuffer();
    p_renderQueue->ClearDynamicCommands();
    p_renderQueue->ClearStaticCommnads();
    p_meshManager->ClearSubmittedModelInstances();
    p_bufferManager->ClearBuffer(TypeFlags::BUFFER_ANIMATION_DATA);

    p_bufferManager->ClearBuffer(TypeFlags::BUFFER_STATIC_MATRIX_DATA);
    p_AnimatedModelManager->ClearSubmittedModelInstances(); */
}

} // namespace eHazGraphics
