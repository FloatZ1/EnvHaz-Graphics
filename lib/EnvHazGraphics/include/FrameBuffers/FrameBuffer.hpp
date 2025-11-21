#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include "DataStructs.hpp"
#include "RenderTexture.hpp"
#include "ShaderManager.hpp"
#include "glad/glad.h"
#include <SDL3/SDL_log.h>
#include <vector>
namespace eHazGraphics {

class FrameBuffer {
private:
  ShaderComboID displayShader;

  unsigned int frameBufferID;
  int m_width, m_height;

  RenderTexture2D m_depthTexture;

  std::vector<RenderTexture2D_Spec> m_colorSpecs;
  RenderTexture2D_Spec m_depthSpec;

  std::vector<RenderTexture2D> m_colorTextures;

  void CreateTexture() {

    m_colorTextures.clear();
    for (auto &spec : m_colorSpecs) {
      m_colorTextures.emplace_back(spec);

      spec.type = m_colorTextures[m_colorTextures.size() - 1].GetSpec().type;
      spec.format =
          m_colorTextures[m_colorTextures.size() - 1].GetSpec().format;
    }

    m_depthTexture = RenderTexture2D(m_depthSpec);

    m_width = m_depthTexture.GetSpec().width;
    m_height = m_depthTexture.GetSpec().height;
  }

  void CreateFrameBuffer() {

    if (frameBufferID)
      glDeleteFramebuffers(1, &frameBufferID);

    glCreateFramebuffers(1, &frameBufferID);

    for (int i = 0; i < m_colorTextures.size(); i++) {

      glNamedFramebufferTexture(frameBufferID, GL_COLOR_ATTACHMENT0 + i,
                                m_colorTextures[i].GetTextureID(), 0);
    }

    if (!m_colorTextures.empty()) {
      std::vector<GLenum> drawBuffers;
      drawBuffers.reserve(m_colorTextures.size());
      for (int i = 0; i < m_colorTextures.size(); ++i)
        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);

      glNamedFramebufferDrawBuffers(frameBufferID, drawBuffers.size(),
                                    drawBuffers.data());
    } else {
      glNamedFramebufferDrawBuffer(frameBufferID, GL_NONE);
    }

    glNamedFramebufferTexture(frameBufferID, GL_DEPTH_ATTACHMENT,
                              m_depthTexture.GetTextureID(), 0);

    GLenum status =
        glCheckNamedFramebufferStatus(frameBufferID, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      SDL_Log("Framebuffer incomplete! Error = 0x%x\n", status);
    }
  }

public:
  FrameBuffer() = default;

  FrameBuffer(int width, int height) : m_width(width), m_height(height) {}

  ~FrameBuffer() {
    if (frameBufferID) {
      glDeleteFramebuffers(1, &frameBufferID);
      for (auto &color : m_colorTextures)
        color.Destroy();

      m_depthTexture.Destroy();
    }
  }
  void SetDisplayShader(ShaderComboID shader) { displayShader = shader; }

  const ShaderComboID &GetShaderID() const { return displayShader; }

  const int &GetWidth() const { return m_width; }
  const int &GetHeight() const { return m_height; }

  void Create(const std::vector<RenderTexture2D_Spec> &colorSpecs,
              const RenderTexture2D_Spec &depthSpec) {
    m_colorSpecs = colorSpecs;
    m_depthSpec = depthSpec;

    m_width = colorSpecs[0].width;
    m_height = colorSpecs[0].height;

    CreateTexture();
    CreateFrameBuffer();
  }

  void Resize(int newWidth, int newHeight) {
    if (newWidth == m_width && newHeight == m_height)
      return;

    m_width = newWidth;
    m_height = newHeight;

    // update specs
    for (auto &spec : m_colorSpecs) {
      spec.width = newWidth;
      spec.height = newHeight;
    }
    m_depthSpec.width = newWidth;
    m_depthSpec.height = newHeight;

    // recreate everything
    CreateTexture();
    CreateFrameBuffer();
  }

  GLuint GetFBO() const { return frameBufferID; }
  const std::vector<RenderTexture2D> &GetColorTextures() const {
    return m_colorTextures;
  }
  const RenderTexture2D &GetDepthTexture() const { return m_depthTexture; }
};

} // namespace eHazGraphics

#endif
