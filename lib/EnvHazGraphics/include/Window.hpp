#ifndef WINDOW_H
#define WINDOW_H




#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_surface.h>
#include <string>



class Window
{


  public:
    bool Create(int width, int height, bool fullscreen);



    void Destroy();



  private:
    int mWidth, mHeight;

    SDL_Window *mWindow = nullptr;
    SDL_Surface *mSurface = nullptr;
};




#endif
