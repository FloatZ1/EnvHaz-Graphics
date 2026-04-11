#ifndef ENVHAZGRAPHICS_HDR_BUFFER_HPP
#define ENVHAZGRAPHICS_HDR_BUFFER_HPP
#include "DataStructs.hpp"
#include "FrameBuffers/FrameBuffer.hpp"
#include "FrameBuffers/RenderTexture.hpp"
#include "glad/glad.h"
#include <cstdint>
namespace eHazGraphics {

class CHDRBuffer {
public:
  CHDRBuffer() {}

  CHDRBuffer(uint32_t width, uint32_t height)
      : m_width(width), m_height(height) {
    m_colorSpec.internalFormat = GL_RGBA16F;
    m_colorSpec.width = width;
    m_colorSpec.height = height;

    // Depth not needed for lighting pass
    m_depthSpec.internalFormat = GL_DEPTH_COMPONENT24;
    m_depthSpec.width = width;
    m_depthSpec.height = height;

    m_framebuffer.Create({m_colorSpec}, m_depthSpec);
  }

  void Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer.GetFBO());
    glViewport(0, 0, m_width, m_height);
    // glClear(GL_COLOR_BUFFER_BIT);
  }

  void BindColor(GLuint slot = 0) {
    glBindTextureUnit(slot, m_framebuffer.GetColorTextures()[0].GetTextureID());
  }

  void Resize(uint32_t width, uint32_t height) {
    if (width == m_width && height == m_height)
      return;

    m_width = width;
    m_height = height;
    m_framebuffer.Resize(width, height);
  }

  FrameBuffer &GetFBO() { return m_framebuffer; }

  uint32_t GetWidth() { return m_width; }
  uint32_t GetHeight() { return m_height; }

private:
  uint32_t m_width, m_height;

  FrameBuffer m_framebuffer;

  RenderTexture2D_Spec m_colorSpec;
  RenderTexture2D_Spec m_depthSpec;
};

} // namespace eHazGraphics

#endif
