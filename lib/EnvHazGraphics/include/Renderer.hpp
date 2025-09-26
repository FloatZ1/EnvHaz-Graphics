#ifndef EnvHazGraphics
#define EnvHazGraphics
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstddef>
#include <lib_export.hpp>
#include <string>


#include "BitFlags.hpp"
#include "Window.hpp"

namespace eHazGraphics
{
// eHazGAPI
class Renderer
{



  public:
    bool shouldQuit = false;



    bool Initialize();



    void SubmitStaticMesh(); // require a an object/container from which to unwrap everything

    void SubmitDynamicData(const void *data, size_t dataSize,
                           TypeFlags dataType); // same, require a container later/ from a octree node or smth



    void PollInputEvents();

    void RenderFrame();

    void UpdateRenderer();



    void Destroy();


  private:
    Window window;
    SDL_Event events;
};



} // namespace eHazGraphics




#endif
