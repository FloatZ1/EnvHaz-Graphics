#ifndef RENDER_TEXTURE_2D_HPP
#define RENDER_TEXTURE_2D_HPP

#include <SDL3/SDL_log.h>
#include <glad/glad.h>

namespace eHazGraphics {

struct RenderTexture2D_Spec {
  int width = -1;
  int height = -1;
  GLenum internalFormat = -1;
  GLenum format = -1;
  GLenum type = -1;
};
class RenderTexture2D {
private:
  RenderTexture2D_Spec t_spec;
  GLuint texture = 0;

public:
  RenderTexture2D() = default;

  RenderTexture2D(const RenderTexture2D_Spec &spec) : t_spec(spec) {
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
      case GL_DEPTH_COMPONENT24:
        t_spec.format = GL_DEPTH_COMPONENT;
        t_spec.type = GL_FLOAT;
        break;
      case GL_DEPTH24_STENCIL8:
        t_spec.format = GL_DEPTH_STENCIL;
        t_spec.type = GL_UNSIGNED_INT_24_8;
        break;
      }
    }
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureStorage2D(texture, 1, t_spec.internalFormat, t_spec.width,
                       t_spec.height);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  GLuint GetTextureID() const { return texture; }
  const RenderTexture2D_Spec &GetSpec() const { return t_spec; }
  void Resize(int newW, int newH) {
    t_spec.width = newW;
    t_spec.height = newH;
    glDeleteTextures(1, &texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureStorage2D(texture, 1, t_spec.internalFormat, t_spec.width,
                       t_spec.height);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

    SDL_Log(
        "\n\n==============FRAME BUFFER TEXTURE %i DESTROYED=============\n\n",
        texture);

    glDeleteTextures(1, &texture);
  }
};

} // namespace eHazGraphics

#endif
