#include "Window.hpp"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>



bool Window::Create(int width, int height, bool fullscreen)
{

    mHeight = height;
    mWidth = width;


    bool success = true;
    if (mWindow = SDL_CreateWindow("Hometown funk", width, height, 0); mWindow == nullptr)

    {
        SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        SDL_Log("IT CREATES LE WINDOW \n\n\n\n\n\n\n");
        ;
        mSurface = SDL_GetWindowSurface(mWindow);
    }


    return success;
}
