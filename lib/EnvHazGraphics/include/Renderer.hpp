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

  void SetCameraPosition(const glm::vec3 &pos) { cameraPosition = pos; }

  glm::mat4 GetViewMatrix() { return ViewMatrix; }
  glm::mat4 GetProjection() { return ProjectionMatrix; }

  void SetViewProjection(glm::mat4 view, glm::mat4 projection) {

    ViewMatrix = view;
    ProjectionMatrix = projection;
  }

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

  void BindGeometryBuffer() { m_gbGeometryBuffer->BindBuffer(); }

  void BindGeometryBufferTextures(uint32_t p_uiDepthSlot = 0) {
    m_gbGeometryBuffer->BindTextures();
    m_gbGeometryBuffer->BindDepth(p_uiDepthSlot);
  }

  void BindHDRBuffer() { m_bHDRBuffer->Bind(); }

  void BindHDRColor(uint32_t p_uiSlot = 0) { m_bHDRBuffer->BindColor(0); }

  void PollInputEvents();

  void ClearRenderCommandBuffer();

  void RenderFrame(std::vector<DrawRange> DrawOrder);

  void SwapBuffers() { SDL_GL_SwapWindow(p_window->GetWindowPtr()); }

  void DrawDebug() { p_debugDrawer->DrawDebug(cameraPosition); }

  void RenderLightingPass();

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

  void UpdateRenderer(float deltaTime);

  // future

  const int &GetViewportWidth() const { return vp_width; }
  const int &GetViewportHeight() const { return vp_height; }

  void Destroy();

private:
  ShaderComboID m_scidHDRshader;
  ShaderComboID m_scidToneShader;

  CGeometryBuffer *m_gbGeometryBuffer;
  CHDRBuffer *m_bHDRBuffer;

  SBufferRange m_brCameraData;
  GLsync m_frameFence = nullptr;
  int vp_width, vp_height;
  FrameBuffer mainFBO;
  SDL_Event events;
  /* Window window;

   ShaderManager shaderManager;
   MaterialManager materialManager;
   MeshManager meshManager;
   RenderQueue renderQueue; */
  // BufferManager bufferManager;
};

} // namespace eHazGraphics

#endif
