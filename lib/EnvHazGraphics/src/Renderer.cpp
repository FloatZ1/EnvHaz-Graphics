#include "Renderer.hpp"
#include "Window.hpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <glad/glad.h>
#include <iostream>


namespace eHazGraphics
{




bool Renderer::Initialize()
{
    bool success{true};

    if (SDL_Init(SDL_INIT_VIDEO) == false)
    {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;
    }



    if (window.Create(200, 200, false, "yummers"))
    {
        SDL_Log("Window created successfully.\n");
        if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        {
            SDL_Log("GLAD INITIALIZED SUCESSFULLY!");
            glViewport(0, 0, window.GetWidth(), window.GetHeight());
            glClearColor(0.1f, 0.5f, 0.7f, 1.0f);
        }
        else
        {
            SDL_Log("Failed to initialize GLAD");
        }
    }



    return success;
    std::cout << "piss\n\n :)))";
}



void Renderer::SubmitStaticMesh()
{
}
void Renderer::SubmitDynamicMesh()
{
}



void Renderer::PollInputEvents()
{
    SDL_PollEvent(&events);
}


void Renderer::RenderFrame()
{
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(window.GetWindowPtr());
}

void Renderer::UpdateRenderer()
{
    PollInputEvents();
    if (events.type == SDL_EVENT_QUIT)
        shouldQuit = true;
}

void Renderer::Destroy()
{
}



} // namespace eHazGraphics
