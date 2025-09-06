
#include "Renderer.hpp"







int main()
{
    eHazGraphics::Renderer rend;
    rend.Initialize();

    while (rend.shouldQuit == false)
    {

        rend.UpdateRenderer();
        rend.RenderFrame();
    }




    return 0;
}
