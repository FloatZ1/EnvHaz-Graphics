#include "Window.hpp"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <glad/glad.h>


bool Window::Create(int width, int height, bool fullscreen, std::string tittle)
{

    mHeight = height;
    mWidth = width;

    // SDL window creation
    bool success = true;
    mWindow = SDL_CreateWindow(tittle.c_str(), width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!mWindow)

    {
        SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        SDL_Log("IT CREATES LE WINDOW \n\n\n\n\n\n\n");



        glContext = SDL_GL_CreateContext(mWindow);

        if (!glContext)
        {

            SDL_Log("ERROR: COULD NOT CREATE OpenGL CONTEXT.");
            success = false;
        }

        SDL_GL_MakeCurrent(mWindow, glContext);
    }
    // creates the sureface as well, nvm we don need it


    return success;
}



void Window::Update()
{
}








SDL_Window *Window::GetWindowPtr() const
{
    return mWindow;
}
SDL_GLContext Window::GetOpenGLContext()
{
    return glContext;
}
int Window::GetWidth() const
{
    return mWidth;
}
int Window::GetHeight() const
{
    return mHeight;
}
