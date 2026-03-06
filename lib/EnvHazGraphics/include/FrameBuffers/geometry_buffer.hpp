#ifndef ENVHAZGRAPHICS_GEOMETRY_BUFFER_HPP
#define ENVHAZGRAPHICS_GEOMETRY_BUFFER_HPP

#include "DataStructs.hpp"
#include "FrameBuffers/FrameBuffer.hpp"
#include "FrameBuffers/RenderTexture.hpp"
#include "glad/glad.h"
#include <cstdint>
namespace eHazGraphics {

class CGeometryBuffer {

public:
  CGeometryBuffer() {};

  CGeometryBuffer(uint32_t p_uiWidth, uint32_t p_uiHeight,
                  ShaderComboID p_scidShader)
      : m_scidShader(p_scidShader), m_uiHeight(p_uiHeight),
        m_uiWidth(p_uiWidth) {

    m_rt2DAlbedo.internalFormat = GL_RGBA8;
    m_rt2DNormal.internalFormat = GL_RGBA16F;
    m_rt2DEmission.internalFormat = GL_RGBA16F;
    m_rt2DPRM.internalFormat = GL_RGBA8;

    m_rt2DDepthSpec.internalFormat = GL_DEPTH_COMPONENT24;

    m_rt2DDepthSpec.height = p_uiHeight;
    m_rt2DDepthSpec.width = p_uiWidth;

    m_rt2DAlbedo.height = p_uiHeight;
    m_rt2DAlbedo.width = p_uiWidth;

    m_rt2DNormal.height = p_uiHeight;
    m_rt2DNormal.width = p_uiWidth;

    m_rt2DEmission.height = p_uiHeight;
    m_rt2DEmission.width = p_uiWidth;

    m_rt2DPRM.height = p_uiHeight;
    m_rt2DPRM.width = p_uiWidth;

    m_fbGBuffer.Create({m_rt2DAlbedo, m_rt2DNormal, m_rt2DPRM, m_rt2DEmission},
                       m_rt2DDepthSpec);

    m_fbGBuffer.SetDisplayShader(p_scidShader);
  }

  void BindTextures(GLuint albedoSlot = 0, GLuint normalSlot = 1,
                    GLuint prmSlot = 2, GLuint emissionSlot = 3) {
    const auto &textures = m_fbGBuffer.GetColorTextures();
    glBindTextureUnit(albedoSlot, textures[0].GetTextureID());
    glBindTextureUnit(normalSlot, textures[1].GetTextureID());
    glBindTextureUnit(prmSlot, textures[2].GetTextureID());
    glBindTextureUnit(emissionSlot, textures[3].GetTextureID());
  }

  void BindBuffer() {

    // glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbGBuffer.GetFBO());
    glViewport(0, 0, m_uiWidth, m_uiHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
  void BindDepth(GLuint slot) {
    glBindTextureUnit(slot, m_fbGBuffer.GetDepthTexture().GetTextureID());
  }

  void Resize(uint32_t p_uiNewWidth, uint32_t p_uiNewHeight) {

    m_fbGBuffer.Resize(p_uiNewWidth, p_uiNewHeight);
    m_uiWidth = p_uiNewWidth;
    m_uiHeight = p_uiNewHeight;

    m_rt2DDepthSpec.height = m_uiHeight;
    m_rt2DDepthSpec.width = m_uiWidth;

    m_rt2DAlbedo.height = m_uiHeight;
    m_rt2DAlbedo.width = m_uiWidth;

    m_rt2DNormal.height = m_uiHeight;
    m_rt2DNormal.width = m_uiWidth;

    m_rt2DEmission.height = m_uiHeight;
    m_rt2DEmission.width = m_uiWidth;

    m_rt2DPRM.height = m_uiHeight;
    m_rt2DPRM.width = m_uiWidth;
  }

private:
  uint32_t m_uiWidth, m_uiHeight;

  ShaderComboID m_scidShader;

  FrameBuffer m_fbGBuffer;

  RenderTexture2D_Spec m_rt2DDepthSpec;
  RenderTexture2D_Spec m_rt2DAlbedo;
  RenderTexture2D_Spec m_rt2DNormal;
  RenderTexture2D_Spec m_rt2DEmission;
  RenderTexture2D_Spec m_rt2DPRM;
};

} // namespace eHazGraphics

#endif
