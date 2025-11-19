#include "Window.hpp"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <cstdlib>
#include <glad/glad.h>

namespace eHazGraphics {

bool Window::Create(int width, int height, bool fullscreen, std::string title) {
  mHeight = height;
  mWidth = width;

  // ---- NEW: set GL attributes BEFORE creating window/context ----
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
  // ----------------------------------------------------------------

  bool success = true;
  mWindow = SDL_CreateWindow(title.c_str(), width, height,
                             SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                 SDL_WINDOW_HIGH_PIXEL_DENSITY);
  if (!mWindow) {
    SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
    success = false;
  } else {
    glContext = SDL_GL_CreateContext(mWindow);

    if (!glContext) {
      SDL_Log("ERROR: COULD NOT CREATE OpenGL CONTEXT.");
      success = false;
    } else {
      SDL_GL_MakeCurrent(mWindow, glContext);
      SDL_Log("OpenGL context created successfully");
    }
  }

  return success;
}

bool previousCursorStat = false;

void Window::Update() {
  // if (lockCursor != previousCursorStat)
  {

    SDL_SetWindowRelativeMouseMode(mWindow, lockCursor);
    // SDL_CaptureMouse(true);
    SDL_WarpMouseInWindow(mWindow, std::abs(mWidth / 2), std::abs(mHeight / 2));

    previousCursorStat = lockCursor;
  }
}

SDL_Window *Window::GetWindowPtr() const { return mWindow; }
SDL_GLContext Window::GetOpenGLContext() { return glContext; }
int Window::GetWidth() const { return mWidth; }
int Window::GetHeight() const { return mHeight; }

} // namespace eHazGraphics
