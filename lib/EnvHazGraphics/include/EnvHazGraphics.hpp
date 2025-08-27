#ifndef EnvHazGraphics
#define EnvHazGraphics
#include <lib_export.hpp>





class eHazGAPI Renderer
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
    void Initialize();



    void SubmitStaticMesh();

    void SubmitDynamicMesh();


    void PollInputEvents();

    void RenderFrame();





    void Destroy();
};








#endif
