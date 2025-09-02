
#include "Renderer.hpp"
#include <iostream>






int main()
{
    Renderer rend;
    rend.Initialize();

    while (rend.shouldQuit == false)
    {

        rend.UpdateRenderer();
        rend.RenderFrame();
    }




    return 0;
}
