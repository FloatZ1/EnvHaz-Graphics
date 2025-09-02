#ifndef EnvHazGraphics
#define EnvHazGraphics
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <lib_export.hpp>
#include <string>


#include "Window.hpp"


// eHazGAPI
class Renderer
{

    // TODO:
    // Add the buffers for the dynamic and static mesh batching plus 2
    // more incase there is instancing for either
    //
    // next up is to make a window class which creates and destroys
    // the window and also handles Input events
    // for the purposes of this library we will asume only one window
    // will be created

  public:
    bool shouldQuit = false;



    bool Initialize();



    void SubmitStaticMesh();

    void SubmitDynamicMesh();



    void PollInputEvents();

    void RenderFrame();

    void UpdateRenderer();



    void Destroy();


  private:
    Window window;
    SDL_Event events;
};








#endif
