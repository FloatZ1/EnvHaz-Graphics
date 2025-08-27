#ifndef WINDOW_H
#define WINDOW_H



#include "lib_export.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_surface.h>
#include <string>



class Window
{


  public:
    Window *Create(int width, int height, bool fullscreen);



    void Destroy();

    ~Window();

  private:
    SDL_Window *mWindow = nullptr;
    SDL_Surface *mSurface = nullptr;
};




#endif
