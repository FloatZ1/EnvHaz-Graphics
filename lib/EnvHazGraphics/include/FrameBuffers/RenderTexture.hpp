#ifndef RENDER_TEXTURE_2D_HPP
#define RENDER_TEXTURE_2D_HPP

#include <SDL3/SDL_log.h>
#include <glad/glad.h>

namespace eHazGraphics {

struct RenderTexture2D_Spec {
  int width = -1;
  int height = -1;
  int layers = 1;

  GLenum internalFormat = static_cast<unsigned int>(-1);
  GLenum format = static_cast<unsigned int>(-1);
  GLenum type = static_cast<unsigned int>(-1);

  GLenum target = GL_TEXTURE_2D;
  bool enableCompare = false;
};
class RenderTexture2D {
private:
  RenderTexture2D_Spec t_spec;
  GLuint texture = 0;

public:
  RenderTexture2D() = default;

  RenderTexture2D(const RenderTexture2D_Spec spec) : t_spec(spec) {
    if (t_spec.format == -1 || t_spec.type == -1) {
      switch (t_spec.internalFormat) {
      case GL_RGBA16F:
        t_spec.format = GL_RGBA;
        t_spec.type = GL_FLOAT;
        break;
      case GL_RGBA8:
        t_spec.format = GL_RGBA;
        t_spec.type = GL_UNSIGNED_BYTE;
        break;
      case GL_DEPTH_COMPONENT32F:
        t_spec.format = GL_DEPTH_COMPONENT;
        t_spec.type = GL_FLOAT;
        break;
      case GL_DEPTH_COMPONENT24:
        t_spec.format = GL_DEPTH_COMPONENT;
        t_spec.type = GL_UNSIGNED_INT;
        break;
      case GL_DEPTH24_STENCIL8:
        t_spec.format = GL_DEPTH_STENCIL;
        t_spec.type = GL_UNSIGNED_INT_24_8;
        break;
      case GL_SRGB8_ALPHA8:
        t_spec.format = GL_RGBA;
        t_spec.type = GL_UNSIGNED_BYTE;
        break;
      }
    }
    GLenum target = (t_spec.layers > 1) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;

    glCreateTextures(target, 1, &texture);
    if (!glIsTexture(texture))
      SDL_Log("Error: Couldnt create Render Texture");
    if (t_spec.layers > 1) {
      // Arrays use 3D storage (Width, Height, Layers)
      glTextureStorage3D(texture, 1, t_spec.internalFormat, t_spec.width,
                         t_spec.height, t_spec.layers);
    } else {
      glTextureStorage2D(texture, 1, t_spec.internalFormat, t_spec.width,
                         t_spec.height);
    }

    // Parameters (S and T are same, but Arrays sometimes use GL_TEXTURE_WRAP_R
    // for the layer)
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // For Shadows, we usually want Border Color to be White (1.0)
    // so objects outside the frustum aren't in shadow
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // SPECIAL FOR SHADOWS: Enable hardware PCF (Percentage Closer Filtering)
    if (t_spec.enableCompare) {
      glTextureParameteri(texture, GL_TEXTURE_COMPARE_MODE,
                          GL_COMPARE_REF_TO_TEXTURE);
      glTextureParameteri(texture, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    } else {
    }
  }
  GLuint GetTextureID() const { return texture; }
  const RenderTexture2D_Spec &GetSpec() const { return t_spec; }
  void Resize(int newW, int newH) {
    t_spec.width = newW;
    t_spec.height = newH;

    GLenum target = (t_spec.layers > 1) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    // Unbind texture and framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(target, 0);

    if (texture)
      glDeleteTextures(1, &texture);

    glCreateTextures(target, 1, &texture);
    if (!glIsTexture(texture))
      SDL_Log("Error: Couldnt create Render Texture");
    if (t_spec.layers > 1) {
      // Arrays use 3D storage (Width, Height, Layers)
      glTextureStorage3D(texture, 1, t_spec.internalFormat, t_spec.width,
                         t_spec.height, t_spec.layers);
    } else {
      glTextureStorage2D(texture, 1, t_spec.internalFormat, t_spec.width,
                         t_spec.height);
    }

    // Parameters (S and T are same, but Arrays sometimes use GL_TEXTURE_WRAP_R
    // for the layer)
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // For Shadows, we usually want Border Color to be White (1.0)
    // so objects outside the frustum aren't in shadow
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // SPECIAL FOR SHADOWS: Enable hardware PCF (Percentage Closer Filtering)
    if ((t_spec.format == GL_DEPTH_COMPONENT ||
         t_spec.format == GL_DEPTH_STENCIL) &&
        t_spec.internalFormat == GL_DEPTH_COMPONENT32F) {
      glTextureParameteri(texture, GL_TEXTURE_COMPARE_MODE,
                          GL_COMPARE_REF_TO_TEXTURE);
      glTextureParameteri(texture, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
  }

  RenderTexture2D(RenderTexture2D &&other) noexcept
      : t_spec(other.t_spec), texture(other.texture) {
    other.texture = 0; // prevent destructor from deleting
  }

  // Move assignment
  RenderTexture2D &operator=(RenderTexture2D &&other) noexcept {
    if (this != &other) {
      if (texture)
        glDeleteTextures(1, &texture);
      t_spec = other.t_spec;
      texture = other.texture;
      other.texture = 0; // prevent destructor from deleting
    }
    return *this;
  }

  // Delete copy constructor/assignment
  RenderTexture2D(const RenderTexture2D &) = delete;
  RenderTexture2D &operator=(const RenderTexture2D &) = delete;

  void Destroy() {

    // SDL_Log(
    //    "\n\n==============FRAME BUFFER TEXTURE %i
    //    DESTROYED=============\n\n", texture);

    glDeleteTextures(1, &texture);
  }
};

} // namespace eHazGraphics

#endif
