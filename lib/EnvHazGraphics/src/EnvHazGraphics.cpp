#include "EnvHazGraphics.hpp"
#include "Window.hpp"
#include <SDL3/SDL_events.h>
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


    SDL_Event e;
    SDL_zero(e);
    bool quit = false;
    while (quit == false)
    {

        SDL_FillSurfaceRect(window.mSurface, nullptr, SDL_MapSurfaceRGB(window.mSurface, 0xFF, 0xFF, 0xFF));



        while (SDL_PollEvent(&e))
        {

            if (e.type == SDL_EVENT_QUIT)
            {
                // End the main loop
                quit = true;
            }
        }


        SDL_UpdateWindowSurface(window.mWindow);
    }


    return success;
    std::cout << "piss\n\n :)))";
}
