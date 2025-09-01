#include "EnvHazGraphics.hpp"
#include "Window.hpp"
#include <iostream>



bool Renderer::Initialize()
{
    bool success{true};

    if (SDL_Init(SDL_INIT_VIDEO) == false)
    {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;
    }



    window.Create(200, 200, false);




    return success;
    std::cout << "piss\n\n :)))";
}
