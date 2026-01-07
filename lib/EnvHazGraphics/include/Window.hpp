#ifndef WINDOW_H
#define WINDOW_H




#include <SDL3/SDL.h>

#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <string>

namespace eHazGraphics
{



class Window
{


  public:
    bool Create(int width, int height, bool fullscreen, std::string tittle);




    void Destroy();

    void Update();

    // Getters


    void ToggleMouseCursor()
    {
        lockCursor = !lockCursor;
    }

    bool isCursorLocked() const
    {
        return lockCursor;
    }

    void SetDimensions(int width, int height)
    {
        mWidth = width;
        mHeight = height;
    }


    SDL_Window *GetWindowPtr() const;

    SDL_Surface *GetSurfacePtr() const;

    int GetWidth() const;

    int GetHeight() const;

    SDL_GLContext GetOpenGLContext();




  private:
    int mWidth, mHeight;
    bool lockCursor = false;
    SDL_Window *mWindow = nullptr;
    SDL_GLContext glContext; // its a typedef to a void*
};

} // namespace eHazGraphics


#endif
