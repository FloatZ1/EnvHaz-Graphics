#ifndef EnvHazGraphics
#define EnvHazGraphics
#include "Utils/Drawing/DebugDrawer.hpp"
#include "glad/glad.h"
#include <SDL3/SDL.h>

#include <cstddef>
#include <future>
#include <lib_export.hpp>
#include <map>
#include <memory>
#include <platform.hpp>
#include <string>
#include <vector>
// temp

#include "Animation/AnimatedModelManager.hpp"
#include "BitFlags.hpp"
#include "BufferManager.hpp"

#include "DataStructs.hpp"
#include "FrameBuffers/FrameBuffer.hpp"
#include "FrameBuffers/HDR_Buffer.hpp"
#include "FrameBuffers/geometry_buffer.hpp"
#include "MaterialManager.hpp"
#include "MeshManager.hpp"
#include "RenderQueue.hpp"
#include "ShaderManager.hpp"
#include "Utils/SingleDrawBuffer.hpp"
#include "Window.hpp"
#include "glm/ext/matrix_float4x4.hpp"
namespace eHazGraphics {
// eHazGAPI

// #define EHAZ_DEBUG
class eHazGAPI Renderer {

public:
  static std::unique_ptr<Renderer> r_instance;

  static std::unique_ptr<Window> p_window;

  static std::unique_ptr<ShaderManager> p_shaderManager;
  static std::unique_ptr<AnimatedModelManager> p_AnimatedModelManager;
  static std::unique_ptr<MaterialManager> p_materialManager;
  static std::unique_ptr<MeshManager> p_meshManager;
  static std::unique_ptr<RenderQueue> p_renderQueue;
  static std::unique_ptr<BufferManager> p_bufferManager;
  static std::unique_ptr<DebugDrawer> p_debugDrawer;

  glm::vec3 cameraPosition;

  glm::mat4 ViewMatrix, ProjectionMatrix;

  float aspect, fov;

  glm::mat4 m_arrShadowMatrices[4];
  float u_CascadeEnds[4];

  uint32_t m_uiNumGI_probes = 0;

  uint32_t m_uiNumGI_grids = 0;

  glm::vec2 GetCurrentFramebufferWH() { return {fb_width, fb_height}; }

  void SetCameraPosition(const glm::vec3 &pos) { cameraPosition = pos; }

  glm::mat4 GetViewMatrix() { return ViewMatrix; }
  glm::mat4 GetProjection() { return ProjectionMatrix; }

  void SetViewProjection(glm::mat4 view, glm::mat4 projection) {

    ViewMatrix = view;
    ProjectionMatrix = projection;
  }

  void SetCameraPlanes(float near, float far) {
    m_fCamNearPlane = near;
    m_fCamFarPlane = far;
  }

  glm::vec2 GetNearFarPlanes() { return {m_fCamNearPlane, m_fCamFarPlane}; }

  const SDL_Event &GetEvent() const { return events; }

  bool shouldQuit = false;
  void SetViewport(int width, int height);
  void SetFrameBuffer(const FrameBuffer &fbo);

  void DefaultFrameBuffer();

  FrameBuffer &GetMainFBO() { return mainFBO; }

  bool Initialize(int width = 1920, int height = 1080, std::string tittle = "",
                  bool fullscreen = false);

  void SubmitStaticModel(ModelID modelID, glm::mat4 position,
                         uint32_t materialID, ShaderComboID usedShader,
                         TypeFlags dataType); // require a an object/container
                                              // from which to unwrap everything
  void SubmitAnimatedModel(ModelID modelID, glm::mat4 position,
                           uint32_t materialID, ShaderComboID usedShader);

  SBufferRange
  SubmitDynamicData(const void *data, size_t dataSize,
                    TypeFlags dataType); // same, require a container later/
                                         // from a octree node or smth

  void UpdateDynamicData(SBufferRange &location, const void *data,
                         const size_t size);

  void BindGeometryBuffer() {
    m_gbGeometryBuffer->BindBuffer();

    fb_width = m_gbGeometryBuffer->GetWidth();
    fb_height = m_gbGeometryBuffer->GetHeight();
  }

  void BindGeometryBufferTextures(uint32_t p_uiDepthSlot = 0) {
    m_gbGeometryBuffer->BindTextures();
    m_gbGeometryBuffer->BindDepth(p_uiDepthSlot);
  }

  void BindHDRBuffer() {
    m_bHDRBuffer->Bind();
    fb_width = m_bHDRBuffer->GetWidth();
    fb_height = m_bHDRBuffer->GetHeight();
  }

  void BindHDRColor(uint32_t p_uiSlot = 0) { m_bHDRBuffer->BindColor(0); }

  void PollInputEvents();

  void ClearRenderCommandBuffer();

  void RenderFrame(std::vector<DrawRange> DrawOrder);

  void SwapBuffers() { SDL_GL_SwapWindow(p_window->GetWindowPtr()); }

  void DrawDebug() { p_debugDrawer->DrawDebug(cameraPosition); }

  void RenderLightingPass();

  void RenderSkyPass();

  void RenderHDRToScreen();

  void EndFrame() {
    if (m_frameFence)
      glDeleteSync(m_frameFence);

    m_frameFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  }

  void WaitForGPU() {
    if (!m_frameFence)
      return;

    glClientWaitSync(m_frameFence, GL_SYNC_FLUSH_COMMANDS_BIT, UINT64_MAX);

    glDeleteSync(m_frameFence);
    m_frameFence = nullptr;
  }

  void DisplayFrameBuffer(const FrameBuffer &fbo) {

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // draw to window
    bool depthVal = glIsEnabled(GL_DEPTH_TEST);

    glDisable(GL_DEPTH_TEST);

    p_shaderManager->UseProgramme(fbo.GetShaderID());

    glBindTextureUnit(0, fbo.GetColorTextures()[0].GetTextureID());

    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (depthVal == true) {
      glEnable(GL_DEPTH_TEST);
    }
  }

  void SetSkyModelScale(float scale) { m_fSkyModelScale = scale; }

  void SetHDRShader(ShaderComboID shader) { m_scidHDRshader = shader; }

  void SetToneShader(ShaderComboID shader) { m_scidToneShader = shader; }

  void SetSkyboxShader(ShaderComboID shader) { m_scidSkyboxShader = shader; }

  void SetVisisbleLightCount(uint32_t p_uiLightCount) {
    m_uiNumLights = p_uiLightCount;
  };

  uint32_t GetVisibleLightCount() { return m_uiNumLights; }

  void UpdateRenderer(float deltaTime);

  // future

  const int &GetViewportWidth() const { return vp_width; }
  const int &GetViewportHeight() const { return vp_height; }

  // Rayleigh Beta coefficients
  const glm::vec3 &GetBetaRayleigh() const { return m_v3BetaRayleigh; }
  // Mie Beta coefficients
  const glm::vec3 &GetBetaMie() const { return m_v3BetaMie; }
  // Ozone Beta coefficients
  const glm::vec3 &GetBetaOzone() const { return m_v3BetaOzone; }

  // Light exposure value
  float GetLightExposure() const { return m_fLightExposure; }
  // Solar brightness intensity
  float GetSolarBrightness() const { return m_fSolarBrightness; }

  // Normalized sun direction vector
  const glm::vec3 &GetSunDirection() const { return m_v3SunDirection; }

  // Rayleigh scale height/factor
  float GetRayLeighScale() const { return m_fRayLeighScale; }
  // Mie scale height/factor
  float GetMieScale() const { return m_fMieScale; }

  // --- Setters ---

  void SetBetaRayleigh(const glm::vec3 &v3BetaRayleigh) {
    m_v3BetaRayleigh = v3BetaRayleigh;
  }
  void SetBetaMie(const glm::vec3 &v3BetaMie) { m_v3BetaMie = v3BetaMie; }
  void SetBetaOzone(const glm::vec3 &v3BetaOzone) {
    m_v3BetaOzone = v3BetaOzone;
  }

  void SetLightExposure(float fLightExposure) {
    m_fLightExposure = fLightExposure;
  }
  void SetSolarBrightness(float fSolarBrightness) {
    m_fSolarBrightness = fSolarBrightness;
  }

  // Note: Usually a good idea to ensure direction remains normalized
  void SetSunDirection(const glm::vec3 &v3SunDirection) {
    m_v3SunDirection = glm::normalize(v3SunDirection);
  }

  void SetRayLeighScale(float fRayLeighScale) {
    m_fRayLeighScale = fRayLeighScale;
  }
  void SetMieScale(float fMieScale) { m_fMieScale = fMieScale; }

  void SetSkyModelShader(ShaderComboID shader) {
    m_scidSkyModelShader = shader;
  }

  void SetSkyModelMaterial(MaterialID top, MaterialID side1, MaterialID side2) {
    m_matSkyModelTop = top;
    m_matSkyModelSide1 = side1;
    m_matSkyModelSide2 = side2;
  }

  void SetSkyModelTop_Material(MaterialID top) { m_matSkyModelTop = top; }

  void SetSkyModelSide1_Material(MaterialID side1) {
    m_matSkyModelSide1 = side1;
  }
  void SetSkyModelSide2_Material(MaterialID side2) {
    m_matSkyModelSide2 = side2;
  }
  void SetSunColor(glm::vec3 sunColor) { m_v3SunColor = sunColor; }

  void SetSunSize(float size) { m_fSunSize = size; }

  void SetSkyModel(ModelID side1, ModelID side2, ModelID top) {
    m_midSkyModelTop = top;
    m_midSkyModelSide1 = side1;
    m_midSkyModelSide2 = side2;

    MeshID l_uiMeshTop_ID =
        p_meshManager->GetModel(m_midSkyModelTop)->GetMeshIDs()[0];
    MeshID l_uiMeshSide1_ID =
        p_meshManager->GetModel(m_midSkyModelSide1)->GetMeshIDs()[0];
    MeshID l_uiMeshSide2_ID =
        p_meshManager->GetModel(m_midSkyModelSide2)->GetMeshIDs()[0];

    auto topModel = p_meshManager->GetModel(l_uiMeshTop_ID);

    auto side1Model = p_meshManager->GetModel(l_uiMeshSide1_ID);

    auto side2Model = p_meshManager->GetModel(l_uiMeshSide2_ID);

    const Mesh &topMesh = p_meshManager->GetMesh(l_uiMeshTop_ID);

    const Mesh &side1Mesh = p_meshManager->GetMesh(l_uiMeshSide1_ID);
    const Mesh &side2Mesh = p_meshManager->GetMesh(l_uiMeshSide2_ID);

    glm::mat4 topMat =
        p_meshManager->GetMesh(l_uiMeshTop_ID).GetRelativeMatrix();
    glm::mat4 sideMat1 =
        p_meshManager->GetMesh(l_uiMeshSide1_ID).GetRelativeMatrix();
    glm::mat4 sideMat2 =
        p_meshManager->GetMesh(l_uiMeshSide2_ID).GetRelativeMatrix();

    m_sdbSkyModelTop_Buffer.SetBufferData(
        p_meshManager->GetMesh(l_uiMeshTop_ID).GetMeshData(), topMat);
    m_sdbSkyModelSide1_Buffer.SetBufferData(
        p_meshManager->GetMesh(l_uiMeshSide1_ID).GetMeshData(), sideMat1);
    m_sdbSkyModelSide2_Buffer.SetBufferData(
        p_meshManager->GetMesh(l_uiMeshSide2_ID).GetMeshData(), sideMat2);
  }

  glm::vec3 GetAmbientSkyColor() const {
    float sunHeight =
        glm::dot(glm::normalize(m_v3SunDirection), glm::vec3(0.0f, 1.0f, 0.0f));

    float dayFactor = glm::clamp((sunHeight + 0.15f) / 1.15f, 0.0f, 1.0f);

    glm::vec3 rayleigh = m_v3BetaRayleigh;
    glm::vec3 mie = m_v3BetaMie;
    glm::vec3 ozone = m_v3BetaOzone;

    // normalize spectral contribution
    glm::vec3 scatterColor =
        glm::normalize(rayleigh * 0.7f + mie * 0.2f + ozone * 0.1f);

    // daytime sky tint
    glm::vec3 daySky = scatterColor;

    // sunset tint
    glm::vec3 sunsetSky =
        glm::mix(glm::vec3(1.0f, 0.45f, 0.20f), scatterColor, dayFactor);

    // night tint
    glm::vec3 nightSky = glm::vec3(0.02f, 0.03f, 0.08f);

    glm::vec3 skyColor =
        glm::mix(nightSky, glm::mix(sunsetSky, daySky, dayFactor), dayFactor);

    float intensity = (m_fLightExposure * 0.03f) + (m_fSolarBrightness * 0.02f);

    return skyColor * intensity;
  }

  void SetCSM_shader(ShaderComboID p_scidCSMshader) {
    m_scidCSMshader = p_scidCSMshader;
  }

  ShaderComboID GetCSM_shader() { return m_scidCSMshader; }

  void RenderShadowMapTextures(std::vector<DrawRange> DrawOrder);

  FrameBuffer &GetShadowFB() { return m_fbShadowCascadeBuffer; }

  glm::vec2 GetCSM_Size() {
    return {(float)m_uiShadowTexWidth, (float)m_uiShadowTexHeight};
  }

  float GetLightDistance() { return m_fLightDistance; }

  void SetLightDistance(float dist) { m_fLightDistance = dist; }

  void Destroy();

private:
  void RenderOnCurrentFrame(std::vector<DrawRange> DrawOrder);

  uint32_t m_uiNumLights = 0; // current frame's number of visible lights for
                              // iteration in the light ssbo.

  GLuint m_uiCurrentBoundFBO = 0;

  float m_fCamNearPlane = 0.1f, m_fCamFarPlane = 100.0f;

  ShaderComboID m_scidHDRshader;
  ShaderComboID m_scidToneShader;
  ShaderComboID m_scidSkyboxShader;
  ShaderComboID m_scidSkyModelShader;

  CGeometryBuffer *m_gbGeometryBuffer;
  CHDRBuffer *m_bHDRBuffer;

  FrameBuffer m_fbShadowCascadeBuffer;
  uint32_t m_uiShadowTexWidth = 2048;
  uint32_t m_uiShadowTexHeight = 2048;
  float m_fLightDistance = 1.0f;

  ShaderComboID m_scidCSMshader;

  SBufferRange m_brCameraData;
  GLsync m_frameFence = nullptr;
  int vp_width, vp_height, fb_width, fb_height;
  FrameBuffer mainFBO;
  SDL_Event events;

  ModelID m_midSkyModelSide1;
  ModelID m_midSkyModelSide2;
  ModelID m_midSkyModelTop;

  eHazGraphics_Utils::CSingleDrawBuffer m_sdbSkyModelTop_Buffer;
  eHazGraphics_Utils::CSingleDrawBuffer m_sdbSkyModelSide1_Buffer;
  eHazGraphics_Utils::CSingleDrawBuffer m_sdbSkyModelSide2_Buffer;

  float m_fSkyModelScale = 1.0f;
  MaterialID m_matSkyModelTop = 0;
  MaterialID m_matSkyModelSide1 = 0;
  MaterialID m_matSkyModelSide2 = 0;

  glm::vec3 m_v3BetaRayleigh = glm::vec3(5.802f, 13.558f, 33.100f);
  glm::vec3 m_v3BetaMie = glm::vec3(3.996f);
  glm::vec3 m_v3BetaOzone = glm::vec3(0.650f, 1.881f, 0.085f);

  float m_fLightExposure = 10.0f;
  float m_fSolarBrightness = 10.0f;

  glm::vec3 m_v3SunDirection = glm::normalize(glm::vec3(0.0f, 0.1f, -1.0f));

  float m_fRayLeighScale = 0.08f;
  float m_fMieScale = 0.012f;
  float m_fSunSize = 0.01f;
  glm::vec3 m_v3SunColor = glm::vec3(1.0f);

  GLuint m_uiCascadeUBO = 0;

  /* Window window;

   ShaderManager shaderManager;
   MaterialManager materialManager;
   MeshManager meshManager;
   RenderQueue renderQueue; */
  // BufferManager bufferManager;
};

} // namespace eHazGraphics

#endif
